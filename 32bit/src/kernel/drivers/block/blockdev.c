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
    block_devices[block_devices_count].device_name[length + 1] = 0; // make sure string is terminated
    block_devices_count++;
    return SUCCESS;
}

bool find_partitions(struct BlockDeviceFile *dev) {
    if (dev->dev.partition != RAW_DEVICE) {
        printf("Device is not suitable for finding partitions\n");
        return FAILURE;
    }

    // Read the first sector into the buffer
    uint8_t buffer[512];
    if (read_sectors_lba(&dev->dev, 0, 1, buffer) == FAILURE) {
        return FAILURE;
    }

    struct PartitionTable* partition_table = (struct PartitionTable*) (buffer + 0x1BE);
    for(char i=0; i<4; i++) {
        struct PartitionTable* entry = partition_table + i;
        if (entry->partition_type == 0) continue; // no partition
        printf("Partition %d: Attr: %x, Type: %x, Start: %d, Size: %d\n", i, entry->drive_attributes, entry->partition_type, entry->lba_partition_start, entry->lba_num_sectors);
        DiskDevice *disk_device = malloc(sizeof(DiskDevice));
        memcpy(&dev->dev, disk_device, sizeof(DiskDevice));
        disk_device->partition = i + 1;
        disk_device->offset = entry->lba_partition_start;

        // The name of the partition will be as per the raw device, plus a dot and a system-generated index
        char device_name[128];
        strcpy(dev->device_name, device_name);
        strcat(device_name, ".");
        register_block_device(disk_device, device_name);
    }

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
        if (strcmp_wl(block_devices[i].device_name, type, length) > 0) {
            count++;
        }
    }
    return count;
}