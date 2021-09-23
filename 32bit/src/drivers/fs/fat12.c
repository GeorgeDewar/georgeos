#include "system.h"

// TODO: In time, we will read these from the disk
#define BYTES_PER_SECTOR        512
#define NUM_RESERVED_SECTORS    2
#define NUMBER_OF_FATS          2
#define SECTORS_PER_FAT         9
#define ROOT_DIR_ENTRIES        224
#define DIR_ENTRY_SIZE          32

#define FAT_0_START             NUM_RESERVED_SECTORS
#define ROOT_DIR_START          NUM_RESERVED_SECTORS + (NUMBER_OF_FATS * SECTORS_PER_FAT)
#define SECTORS_PER_DIR         (ROOT_DIR_ENTRIES * DIR_ENTRY_SIZE) / BYTES_PER_SECTOR
#define DIR_ENTRIES_PER_SECTOR  BYTES_PER_SECTOR / DIR_ENTRY_SIZE
#define DATA_START              32 // FAT0, FAT1, ROOTDIR, 2 special clusters

typedef struct {
    int8_t name[8];                                                                         // 00
    int8_t extension[3];                                                                    // 08

    // Bit field 
    uint8_t readOnly       : 1;                                                             // 11
    uint8_t hidden         : 1;
    uint8_t system         : 1;
    uint8_t volumeLabel    : 1;
    uint8_t subdirectory   : 1;
    uint8_t archive        : 1;
    uint8_t device         : 1;
    uint8_t                : 1;

    uint8_t unused_0C; // 0x0C; CP/M-86 and DOS Plus store user attributes here             // 12
    uint8_t unused_0D; // 0x0D; Various purposes under some DOS versions                    // 13
    uint16_t unused_0E;  // 0x0E; Various purposes under some DOS versions                  // 14
    uint16_t unused_10;  // 0x10; Various purposes under some DOS versions                  // 16
    uint16_t unused_12;                                                                     // 18
    uint16_t unused_14;                                                                     // 20
    uint16_t lastModifiedTime;                                                              // 22
    uint16_t lastModifiedDate;                                                              // 24

    uint16_t startOfFile;                                                                   // 26
    uint32_t fileSize;                                                                      // 28-32
} __attribute__((packed)) Fat12DirectoryEntry;

// Todo: malloc these per FS or as needed
uint8_t *fat;

/**
 * Header section
 */
bool fat12_init(DiskDeviceDriver* device, uint8_t device_num);
bool fat12_list_dir(DiskDeviceDriver* device, uint8_t device_num, char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
bool fat12_read_file(DiskDeviceDriver* device, uint8_t device_num, char* path, uint8_t* buffer, uint16_t* length_out);
bool find_file(DiskDeviceDriver* device, uint8_t device_num, char* path, DirEntry* dir_entry_out);
static bool fat12_read_fat(DiskDeviceDriver* device, uint8_t device_num);
static uint16_t fat12_decode_fat_entry(uint16_t cluster_num, uint8_t *fat);

FileSystemDriver fs_fat12 = {
    init: &fat12_init,
    list_dir: &fat12_list_dir,
    read_file: &fat12_read_file
};

/**
 * Implementation
 */

bool fat12_init(DiskDeviceDriver* device, uint8_t device_num) {
    // Allocate buffer to store File Allocation Table
    fat = malloc(SECTORS_PER_FAT * BYTES_PER_SECTOR);
    fat12_read_fat(device, device_num);
    return SUCCESS;
}

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool fat12_list_dir(DiskDeviceDriver* device, uint8_t device_num, char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    Fat12DirectoryEntry dir_entry_buffer[ROOT_DIR_ENTRIES];
    // Assume root dir for now, and just read one sector
    read_sectors_lba(device, device_num, ROOT_DIR_START, SECTORS_PER_DIR, dir_entry_buffer);
    for (int i=0; i<ROOT_DIR_ENTRIES; i++) {
        Fat12DirectoryEntry fat12entry = dir_entry_buffer[i];

        if (fat12entry.name[0] == 0) {
            *num_entries_out = i;
            return SUCCESS; // no more entries
        }
        // Copy filename
        char* filename = fat12entry.name;
        char* extension = fat12entry.extension;
        uint8_t index1;
        uint8_t bufferIndex = 0;
        // Copy the filename part
        index1 = 0;
        while(filename[index1] != ' ' && filename[index1] != 0) {
            dir_entry_list_out[i].filename[bufferIndex++] = filename[index1++];
        }
        // Append a dot to the filename
        dir_entry_list_out[i].filename[bufferIndex++] = '.';
        // Copy the extension
        index1 = 0;
        while(extension[index1] != ' ' && extension[index1] != 0) {
            dir_entry_list_out[i].filename[bufferIndex++] = extension[index1++];
        }
        // Append a NULL byte
        dir_entry_list_out[i].filename[bufferIndex] = 0;
        // Copy flags
        dir_entry_list_out[i].archive = fat12entry.archive;
        dir_entry_list_out[i].read_only = fat12entry.readOnly;
        dir_entry_list_out[i].directory = fat12entry.subdirectory;
        dir_entry_list_out[i].hidden = fat12entry.hidden;
        dir_entry_list_out[i].system = fat12entry.system;
        // Copy filesize
        dir_entry_list_out[i].file_size = fat12entry.fileSize;
        dir_entry_list_out[i].location_on_disk = fat12entry.startOfFile;
    }
    *num_entries_out = ROOT_DIR_ENTRIES;
    return SUCCESS;
}

/** Read an entire file into the supplied buffer, and set length_out */
bool fat12_read_file(DiskDeviceDriver* device, uint8_t device_num, char* path, uint8_t* buffer, uint16_t* length_out) {
    int cluster_index = 1; // we print the first cluster outside the loop
    int read_result;

    // Find the first cluster number from the directory entry
    DirEntry dir_entry;
    if (!find_file(device, device_num, path, &dir_entry)) {
        fprintf(stderr, "File not found\n");
        return FAILURE;
    }

    uint16_t cluster = dir_entry.location_on_disk;

    fprintf(stddebug, "Reading cluster\n");
    if (!read_sectors_lba(device, device_num, cluster + DATA_START, 1, buffer)) {
        fprintf(stderr, "Read failure\n");
        return FAILURE;
    }

    for(;;) {
        fprintf(stddebug, "Reading cluster\n");
        cluster = fat12_decode_fat_entry(cluster, fat);
        if(cluster == 0xFFF) break; // we've read the last cluster already

        if (!read_sectors_lba(device, device_num, cluster + DATA_START, 1, buffer + (BYTES_PER_SECTOR * cluster_index))) {
            fprintf(stderr, "Read failure\n");
            return FAILURE;
        }

        cluster_index++;
    }

    *length_out = dir_entry.file_size;
    return SUCCESS;
}

/**
 * Private functions
 */

/** Read the whole FAT into a buffer */
static bool fat12_read_fat(DiskDeviceDriver* device, uint8_t device_num) {
    return read_sectors_lba(device, device_num, FAT_0_START, SECTORS_PER_FAT, fat);
}

/** Return the location of the next cluster, if any, given a cluster number */
static uint16_t fat12_decode_fat_entry(uint16_t cluster_num, uint8_t *fat) {
    unsigned int offset;
    unsigned char b1, b2;
    
    /* this involves some really ugly bit shifting.  This probably
       only works on a little-endian machine. */
    offset = (3 * (cluster_num / 2));

    if (cluster_num % 2 == 0) {
        b1 = *(fat + offset);
        b2 = *(fat + offset + 1);
        return ((0x0f & b2) << 8) | b1;
    }
    else {
        b1 = *(fat + offset + 1);
        b2 = *(fat + offset + 2);
        return b2 << 4 | ((0xf0 & b1) >> 4);
    }
}

/*
 * Find a file and return the location of its directory entry
 * 
 * Returns -1 if the file cannot be found
 */
bool find_file(DiskDeviceDriver* device, uint8_t device_num, char* path, DirEntry* dir_entry_out) {
    if (*path++ != '/') {
        fprintf(stderr, "Not a valid path\n");
        return FAILURE;
    }
    DirEntry dir_entry_list[ROOT_DIR_ENTRIES];
    uint16_t num_entries;
    
    // Read the root directory
    bool list_res = fat12_list_dir(device, device_num, path, dir_entry_list, &num_entries);
    
    // Loop through the files
    for(uint16_t i=0; i<num_entries; i++) {
        if (strcmp(dir_entry_list[i].filename, path)) {
            *dir_entry_out = dir_entry_list[i];
            return SUCCESS;
        }
    }

    return FAILURE;
}
