#include "system.h"

struct FileSystemMount fs_mounts[MAX_MOUNTPOINTS];
int fs_mounts_count = 0;

char cwd[256];
FileHandle open_files[MAX_FILES];

static bool find_file(FileSystem* fs, char* path, DirEntry* dir_entry_out);
static int next_file_handle();
static int get_next_index_for_mount_type(const char* mount_point_name);
static bool list_root(DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
static bool list_dev(DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
static void write_virtual_dirent(DirEntry* dir_entry, char* name);

/**
 * Add a FileSystem to the list of mounts, so that files in it can be found under /{name}/path/filename
 */
bool mount_fs(FileSystem *fs, const char* mount_point_name) {
    int mount_point_name_length = strlen(mount_point_name);
    fs_mounts[fs_mounts_count].fs = *fs;
    strcpy(mount_point_name, fs_mounts[fs_mounts_count].mount_point);
    int index = get_next_index_for_mount_type(mount_point_name);
    if (index >= MAX_MOUNTPOINTS) return FAILURE;
    fs_mounts[fs_mounts_count].mount_point[mount_point_name_length] = (char) ('0' + index);
    fs_mounts[fs_mounts_count].mount_point[mount_point_name_length + 1] = 0; // terminate string
    fs_mounts_count++;
    return SUCCESS;
}

static int get_next_index_for_mount_type(const char* mount_point_name) {
    int count = 0;
    int mount_point_name_length = strlen(mount_point_name);
    for (int i=0; i<fs_mounts_count; i++) {
        if (strcmp_wl(fs_mounts[fs_mounts_count].mount_point, mount_point_name, mount_point_name_length) > 0) {
            count++;
        }
    }
    return count;
}

bool get_fs_for_path(char* path, char** rest_of_path, FileSystem ** fs_out) {
    if (*path++ != '/') {
        return FAILURE;
    }

    // Find the mountpoint component
    char mountpoint[MOUNTPOINT_LENGTH] = {0};
    int i;
    for(i=0; path[i] != '/'; i++) {
        if (path[i] == 0) {
            fprintf(stddebug, "Found end-of-string before next /\n");
            return FAILURE;
        }
        mountpoint[i] = path[i];
    }
    path += i + 1; // also skip past the slash after the mountpoint

    // Look for the mount entry
    for(i=0; i<fs_mounts_count; i++) {
        if (strcmp(fs_mounts[i].mount_point, mountpoint) > 0) {
            *fs_out = &fs_mounts[i].fs;
            *rest_of_path = path;
            return SUCCESS;
        }
    }

    fprintf(stddebug, "No mountpoint found\n");
    return FAILURE;
}

bool normalise_path(const char* path_in, char* path_out) {
    if (path_in[0] == '/') {
        // path is already absolute
        strcpy(path_in, path_out);
        return SUCCESS;
    }
    if (strlen(path_in) == 0 && strcmp(cwd, "/") != 0) {
        // Special case for root
        strcpy(cwd, path_out);
        return SUCCESS;
    }

    strcpy(cwd, path_out);
    strcat(path_out, "/");
    strcat(path_out, path_in);
    return SUCCESS;
}

bool read_sectors_lba(DiskDevice* device, uint32_t lba, uint32_t count, void* buffer) {
    fprintf(stddebug, "Reading from sector %d for %d\n", lba, count);
    if (device->type == FLOPPY) {
        // TODO: Move this loop to the floppy driver or improve the driver to support multi-sector reads
        for (uint32_t i=0; i<count; i++) {
            if (device->driver->read_sectors(device, lba + i, 1, buffer + (512 * i)) == FAILURE) {
                return FAILURE;
            }
        }
        return SUCCESS;
    } else {
        return device->driver->read_sectors(device, lba, count, buffer);
    }
}

int open_file(char* path) {
    int fp = next_file_handle();
    if (fp == NULL) return -1;

    // Normalise path (make absolute)
    char normalised_path[256];
    normalise_path(path, normalised_path);
    fprintf(stddebug, "Normalised path: %s\n", normalised_path);

    if (strcmp_wl(normalised_path, "/dev/", 5)) {
        // Path refers to a block device
        char *block_name = normalised_path + 5;
        DiskDevice *device = NULL;
        for (int i=0; i<block_devices_count; i++) {
            if (strcmp(block_devices[i].device_name, block_name) != 0) {
                device = &block_devices[i].dev;
                break;
            }
        }
        if (device == NULL) {
            printf("Could not find block device %s\n", block_name);
            return FAILURE;
        }
        // Create the file handle
        FileHandle* handle = &open_files[fp];
        handle->type = BLOCK;
        handle->block_descriptor.block_device = device;
        handle->block_descriptor.cursor = 0;
        strcpy(path, handle->block_descriptor.path);

        return fp;
    }

    // Find filesystem instance
    FileSystem* fs;
    char* fs_path;
    if (get_fs_for_path(normalised_path, &fs_path, &fs) == FAILURE) return FAILURE;

    // Find file in FS
    DirEntry dir_entry;
    if (find_file(fs, fs_path, &dir_entry) < 0) {
        fprintf(stddebug, "Did not find file\n");
        return -1;
    }
    fprintf(stddebug, "Found file\n");

    // Create the file handle
    FileHandle* handle = &open_files[fp];
    handle->type = FILE;
    handle->file_descriptor.filesystem = fs;
    handle->file_descriptor.location_on_disk = dir_entry.location_on_disk;
    handle->file_descriptor.size = dir_entry.file_size;
    strcpy(path, handle->file_descriptor.path);

    return fp;
}

void close_file(int fd) {
    open_files[fd].type = NULL;
}

static int next_file_handle() {
    for(int i=0; i<MAX_FILES; i++) {
        if(open_files[i].type == NULL) return i;
    }
    return -1; // no free file handles
}

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool list_dir(char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    // Normalise path (make absolute)
    char normalised_path[256];
    normalise_path(path, normalised_path);
    fprintf(stddebug, "Resolved path: %s\n", normalised_path);

    if (strcmp(normalised_path, "/") != 0) {
        fprintf(stddebug, "Listing VFS root\n");
        return list_root(dir_entry_list_out, num_entries_out);
    } else if (strcmp(normalised_path, "/dev/") != 0) {
        fprintf(stddebug, "Listing devices\n");
        return list_dev(dir_entry_list_out, num_entries_out);
    }

    // Find filesystem instance
    FileSystem* fs;
    char* fs_path;
    bool res = get_fs_for_path(normalised_path, &fs_path, &fs);
    if (res == FAILURE) {
        fprintf(stderr, "Could not find a filesystem for the path\n");
        return FAILURE;
    }

    // List the files
    return fs->driver->list_root(fs, dir_entry_list_out, num_entries_out);
}

/** Read an entire file into the supplied buffer, and set length_out */
// TODO: Get rid of this and just use open/read/close? Or make read_file_fully wrapper?
bool read_file(char* path, uint8_t* buffer, uint16_t* length_out) {
    int fd = open_file(path);
    if (fd < 0) {
        printf("Failed to open file\n");
        return FAILURE;
    }
    int result = read(fd, buffer, 10000);
    if (result == FAILURE) return FAILURE;
    *length_out = result;
    close_file(fd);
    return SUCCESS;
}

/*
 * Find a file and return the location of its directory entry
 *
 * Returns -1 if the file cannot be found
 */
static bool find_file(FileSystem* fs, char* path, DirEntry* dir_entry_out) {
    DirEntry dir_entry_list[16];
    uint16_t num_entries;

    // Process the path
    char path_copy[256];
    strcpy(path, path_copy);
    if (!fs->case_sensitive) {
        // Up-case the path so that it will match the stored-uppercase one on disk
        strupr(path_copy);
    }

    // Read the directory
    bool list_res = fs->driver->list_root(fs, dir_entry_list, &num_entries);
    if (!list_res) return FAILURE;

    // Loop through the files
    for(uint16_t i=0; i<num_entries; i++) {
        if (strcmp(dir_entry_list[i].filename, path_copy) != 0) {
            *dir_entry_out = dir_entry_list[i];
            return SUCCESS;
        }
    }

    return FAILURE;
}

static bool list_root(DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    write_virtual_dirent(&dir_entry_list_out[0], "dev");
    for (int i=0; i<fs_mounts_count; i++) {
        write_virtual_dirent(&dir_entry_list_out[i+1], fs_mounts[i].mount_point);
    }
    *num_entries_out = 1 + fs_mounts_count;
    return SUCCESS;
}

static bool list_dev(DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    for (int i=0; i<block_devices_count; i++) {
        write_virtual_dirent(&dir_entry_list_out[i], block_devices[i].device_name);
    }
    *num_entries_out = block_devices_count;
    return SUCCESS;
}

static void write_virtual_dirent(DirEntry* dir_entry, char* name) {
    dir_entry->directory = true;
    dir_entry->file_size = 0;
    dir_entry->location_on_disk = 0;
    strcpy(name, dir_entry->filename);
}
