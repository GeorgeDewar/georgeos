#include "system.h"

char cwd[256];

static bool find_file(DiskDevice* device, char* path, DirEntry* dir_entry_out);

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

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool list_dir(char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    DiskDevice device;
    // Get the device
    if(!get_device_for_path(path, &device)) {
        return FAILURE;
    };
    // List the files
    return fs_fat12.list_dir(&device, path, dir_entry_list_out, num_entries_out);
}

/** Read an entire file into the supplied buffer, and set length_out */
bool read_file(char* path, uint8_t* buffer, uint16_t* length_out) {
    DiskDevice device;
    // Get the device
    if(!get_device_for_path(path, &device)) {
        return FAILURE;
    };
    DirEntry dir_entry;
    if(!find_file(&device, path, &dir_entry)) {
        return FAILURE;
    }
    // List the files
    if(!fs_fat12.read_file(&device, dir_entry.location_on_disk, buffer)) {
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
    if (*path++ != '/') {
        fprintf(stderr, "Not a valid path\n");
        return FAILURE;
    }
    DirEntry dir_entry_list[16];
    uint16_t num_entries;

    // Read the root directory
    bool list_res = fs_fat12.list_dir(device, path, dir_entry_list, &num_entries);
    if (!list_res) return FAILURE;

    // Loop through the files
    for(uint16_t i=0; i<num_entries; i++) {
        if (strcmp(dir_entry_list[i].filename, path) != 0) {
            *dir_entry_out = dir_entry_list[i];
            return SUCCESS;
        }
    }

    return FAILURE;
}
