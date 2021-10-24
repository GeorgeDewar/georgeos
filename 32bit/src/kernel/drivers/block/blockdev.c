#include "system.h"
// Generic handling of block devices (drives) as a list of named file-like things

struct BlockDeviceFile block_devices[MAX_BLOCK_DEVICES];
int block_devices_count = 0;

static int get_next_index_for_block_dev_type(const char* type);

/**
 * Add a block device to the list of devices, so it will appear under /dev/{type}{index}, e.g. /dev/ide0
 * @param dev the device to register
 * @param type e.g. "floppy", "ide"
 * @return true if device was registered successfully
 */
bool register_block_device(DiskDevice *dev, char* type) {
    int length = strlen(type);
    block_devices[block_devices_count].dev = *dev;
    strcpy(type, block_devices[block_devices_count].device_name);
    int index = get_next_index_for_block_dev_type(type);
    if (index >= MAX_BLOCK_DEVICES) return FAILURE;
    block_devices[block_devices_count].device_name[length] = (char) ('0' + index);
    block_devices_count++;
    return SUCCESS;
}

/**
 * Return the next index number required to make a unique device name for the specified type. For example, if the type
 * was "floppy" and there existed devices "floppy0" and "floppy1", this function will return 2 as the next index.
 */
static int get_next_index_for_block_dev_type(const char* type) {
    int count = 0;
    int length = strlen(type);
    for (int i=0; i<block_devices_count; i++) {
        if (strcmp_wl(block_devices[block_devices_count].device_name, type, length) > 0) {
            count++;
        }
    }
    return count;
}