#include "system.h"

char cwd[256];

bool get_device_for_path(char* path, DiskDeviceDriver* device_out, uint8_t* device_num_out) {
    // /fd0 = floppy 0, etc, may need lookup table, but for now...
    *device_out = disk_device_floppy;
    *device_num_out = 0;
    return SUCCESS;
}

// array of disks
bool read_sectors_lba(DiskDeviceDriver* device, uint8_t device_num, uint32_t lba, uint32_t count, void* buffer) {
    for (uint32_t i=0; i<count; i++) {
        device->read_sector(device_num, lba + i, buffer + (512 * i));
    }
    return SUCCESS;
}

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool list_dir(char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    DiskDeviceDriver device;
    uint8_t device_num;
    // Get the device
    if(!get_device_for_path(path, &device, &device_num)) {
        return FAILURE;
    };
    // List the files
    return fs_fat12.list_dir(&device, device_num, path, dir_entry_list_out, num_entries_out);
}

/** Read an entire file into the supplied buffer, and set length_out */
bool read_file(char* path, uint8_t* buffer, uint16_t* length_out) {
    DiskDeviceDriver device;
    uint8_t device_num;
    // Get the device
    if(!get_device_for_path(path, &device, &device_num)) {
        return FAILURE;
    };
    // List the files
    return fs_fat12.read_file(&device, device_num, path, buffer, length_out);
}
