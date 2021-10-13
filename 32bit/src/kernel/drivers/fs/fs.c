#include "system.h"

#define MOUNTPOINT_LENGTH   64
#define MAX_MOUNTPOINTS     16
struct FileSystemMount {
    char mount_point[MOUNTPOINT_LENGTH];
    FileSystem fs;
};
struct FileSystemMount fs_mounts[MAX_MOUNTPOINTS];
int fs_mounts_count = 0;

char cwd[256];
FileHandle open_files[MAX_FILES];

static bool find_file(DiskDevice* device, char* path, DirEntry* dir_entry_out);
static int next_file_handle();
static int get_next_index_for_mount_type(const char* mount_point_name);

/**
 * File system master plan:
 *
 * 1. File system drivers implement an identify(Device*) method which returns a boolean indicating if a drive has that
 *    FS type
 * 2. For each detected disk device, find the file system by calling identify on it. When found, initialise an instance
 *    of it. Add that to the disk device table, which maps e.g. /floppy0 to the relevant filesystem (which points to the
 *    device).
 * 3. open() calls normalise_path(char* path) to get an absolute path if not already absolute
 * 4. open() calls get_fs(char* path) which takes an absolute path and identifies the filesystem before stripping that
 *    part of the path
 * 5. open() calls find_file(FileSystem* fs, char* path) to look for the file in the FS
 */

/*
 * Additional notes: For most disks there will be a partition table which we must read. Add LBA offset of partition to
 * the FileSystem struct.
 * For floppy, FAT12 is the only FS.
 */

/**
 * Add a FileSystem to the list of mounts, so that files in it can be found under /{name}/path/filename
 */
bool mount_fs(FileSystem *fs, const char* mount_point_name) {
    int mount_point_name_length = strlen(mount_point_name);
    fs_mounts[fs_mounts_count].fs = *fs;
    strcpy(mount_point_name, fs_mounts[fs_mounts_count].mount_point);
    int index = get_next_index_for_mount_type(mount_point_name);
    fs_mounts[fs_mounts_count].mount_point[mount_point_name_length] = (char) ('0' + index);
    fs_mounts_count++;
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

bool get_device_for_path(char* path, DiskDevice* device_out) {
    // /fd0 = floppy 0, etc, may need lookup table, but for now...
    *device_out = floppy0;
    return SUCCESS;
}

// array of disks
bool read_sectors_lba(DiskDevice* device, uint32_t lba, uint32_t count, void* buffer) {
    for (uint32_t i=0; i<count; i++) {
        device->driver->read_sector(device, lba + i, buffer + (512 * i));
    }
    return SUCCESS;
}

int open_file(char* path) {
    int fp = next_file_handle();
    if (fp == NULL) return -1;

    // Find device
    DiskDevice device;
    get_device_for_path(path, &device);

    // Find file in FS
    DirEntry dir_entry;
    if (find_file(&device, path, &dir_entry) < 0) {
        return -1;
    }

    // Create the file handle
    FileHandle* handle = &open_files[fp];
    handle->type = FILE;
    handle->file_descriptor.filesystem = &floppy0_fs;
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
    DiskDevice device;
    // Get the device
    if(get_device_for_path(path, &device) < 0) {
        return FAILURE;
    };
    // List the files
    return fs_fat12.list_dir(&device, path, dir_entry_list_out, num_entries_out);
}

/** Read an entire file into the supplied buffer, and set length_out */
// TODO: Get rid of this and just use open/read/close? Or make read_file_fully wrapper?
bool read_file(char* path, uint8_t* buffer, uint16_t* length_out) {
    DiskDevice device;
    // Get the device
    if(get_device_for_path(path, &device) < 0) {
        return FAILURE;
    };
    DirEntry dir_entry;
    if(find_file(&device, path, &dir_entry) < 0) {
        return FAILURE;
    }
    // List the files
    FileSystem *fs = &floppy0_fs;
    if(fs->driver->read_file(fs, dir_entry.location_on_disk, buffer) < 0) {
        return FAILURE;
    }
    *length_out = dir_entry.file_size;
    return SUCCESS;
}

/*
 * Find a file and return the location of its directory entry
 *
 * Returns -1 if the file cannot be found
 */
static bool find_file(DiskDevice* device, char* path, DirEntry* dir_entry_out) {
//    if (*path++ != '/') {
//        fprintf(stderr, "Not a valid path\n");
//        return FAILURE;
//    }
    // Identify the filesystem
    FileSystem *fs = &floppy0_fs;

    DirEntry dir_entry_list[16];
    uint16_t num_entries;

    // Read the root directory
    char path_copy[256];
    strcpy(path, path_copy);
    if (!fs->case_sensitive) {
        // Up-case the path so that it will match the stored-uppercase one on disk
        strupr(path_copy);
    }

    bool list_res = fs->driver->list_dir(fs->device, path_copy, dir_entry_list, &num_entries);
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
