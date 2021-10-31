#include "system.h"
#include "fs.h"

#define BYTES_PER_SECTOR        512
#define DIR_ENTRY_SIZE          32

typedef struct {
    char name[8];                                                                           // 00
    char extension[3];                                                                      // 08

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
    uint16_t fat32_start_of_file_high;                                                      // 20
    uint16_t lastModifiedTime;                                                              // 22
    uint16_t lastModifiedDate;                                                              // 24

    uint16_t start_of_file_low;                                                             // 26
    uint32_t file_size;                                                                     // 28-32
} __attribute__((packed)) FatDirectoryEntry;
typedef struct {
    void *fat;
    BiosParameterBlock *bpb;
    char fat_type;
} FatInstanceData;
enum FatType {
    FAT12 = 0,
    FAT16,
    FAT32,
};

/**
 * Header section
 */
bool fat12_init(DiskDevice* device, FileSystem* filesystem_out);
bool fat12_list_dir(FileSystem* fs, char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
bool fat12_read_file(FileSystem * fs, uint32_t cluster, void* buffer);
static bool fat12_read_fat(DiskDevice* device, BiosParameterBlock *bpb, uint8_t *fat);
static uint32_t fat12_decode_fat_entry(uint32_t cluster_num, FileSystem *fs);
static FatInstanceData * instance_data(FileSystem *fs);
static int root_dir_start(FileSystem *fs);
static int first_data_sector(FileSystem *fs);
static bool cluster_is_end_of_chain(uint32_t cluster, FileSystem *fs);

FileSystemDriver fs_fat12 = {
    .init = &fat12_init,
    .list_dir = &fat12_list_dir,
    .read_file = &fat12_read_file
};

/**
 * Implementation
 */

bool fat12_init(DiskDevice* device, FileSystem* filesystem_out) {
    // Read the BPB - return promptly if not FAT12
    uint8_t buffer[BYTES_PER_SECTOR];
    device->driver->read_sectors(device, 0, 1, buffer);
    BiosParameterBlock *bpb = malloc(sizeof(BiosParameterBlock));
    memcpy(buffer + 11, bpb, sizeof(BiosParameterBlock));

    char fat_type;
    if (strcmp_wl(bpb->file_system_type, "FAT12", 5) > 0) {
        fat_type = FAT12;
    } else if (strcmp_wl(bpb->file_system_type, "FAT16", 5) > 0) {
        fat_type = FAT16;
    } else if (strcmp_wl(bpb->file_system_type, "FAT32", 5) > 0) {
        fat_type = FAT32;
    } else {
        fprintf(stddebug, "Not a FAT volume\n");
        return FAILURE;
    }
    bpb->file_system_type[5] = 0; // NULL-terminate the FS type so we can use it as a string

    // Allocate buffer to store File Allocation Table
    uint8_t *fat = malloc(bpb->sectors_per_fat * BYTES_PER_SECTOR);

    // Allocate memory for instance data structure
    FatInstanceData *instance_data = malloc(sizeof(FatInstanceData));
    instance_data->bpb = bpb;
    instance_data->fat = fat;
    instance_data->fat_type = fat_type;

    // Populate the output fields
    filesystem_out->driver = &fs_fat12;
    filesystem_out->device = device;
    filesystem_out->instance_data = instance_data;
    filesystem_out->case_sensitive = false;

    // Print debug info
    fprintf(stddebug, "Initialising %s volume\n", bpb->file_system_type);
    fprintf(stddebug, "  Reserved sectors: %d\n", bpb->num_reserved_sectors);
    fprintf(stddebug, "  Sectors per FAT: %d\n", bpb->sectors_per_fat);
    fprintf(stddebug, "  Number of FATs: %d\n", bpb->num_fats);
    fprintf(stddebug, "  Root dir entries: %d\n", bpb->max_root_directory_entries);

    // Print user-readable message
    char label[12];
    memcpy(bpb->partition_volume_label, label, 11);
    label[11] = 0;
    printf("Mounting %s filesystem with label %s\n", bpb->file_system_type, label);

    // Read the FAT
    if (fat12_read_fat(device, bpb, fat) == FAILURE) return FAILURE;
    return SUCCESS;
}

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool fat12_list_dir(FileSystem* fs, char* path, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    BiosParameterBlock *bpb = ((FatInstanceData *) fs->instance_data)->bpb;
    int sectors_per_dir = bpb->max_root_directory_entries * DIR_ENTRY_SIZE / BYTES_PER_SECTOR;
    FatDirectoryEntry dir_entry_buffer[bpb->max_root_directory_entries];
    // Assume root dir for now, and just read one sector
    read_sectors_lba(fs->device, root_dir_start(fs), sectors_per_dir, dir_entry_buffer);
    for (int i=0; i<bpb->max_root_directory_entries; i++) {
        // printf("Checking entry %d\n", i);
        FatDirectoryEntry fat12entry = dir_entry_buffer[i];

        if (fat12entry.name[0] == 0) {
            *num_entries_out = i;
            fprintf(stddebug, "No more entries (found %d files)\n", i);
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
        if (fat12entry.subdirectory == 0) {
            // Append a dot to the filename
            dir_entry_list_out[i].filename[bufferIndex++] = '.';
            // Copy the extension
            index1 = 0;
            while (extension[index1] != ' ' && extension[index1] != 0) {
                dir_entry_list_out[i].filename[bufferIndex++] = extension[index1++];
            }
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
        dir_entry_list_out[i].file_size = fat12entry.file_size;
        // For FAT32, use this value as the high part of the first cluster - otherwise, don't use it as it may be used
        // for a different purpose
        uint16_t start_of_file_high = instance_data(fs)->fat_type == FAT32 ? fat12entry.fat32_start_of_file_high : 0;
        dir_entry_list_out[i].location_on_disk = fat12entry.start_of_file_low + (start_of_file_high << 16);
    }
    *num_entries_out = bpb->max_root_directory_entries;
    return SUCCESS;
}

/** Read an entire file into the supplied buffer, and set length_out */
bool fat12_read_file(FileSystem * fs, uint32_t cluster, void* buffer) {
    int data_start = first_data_sector(fs);
    int sectors_per_cluster = instance_data(fs)->bpb->sectors_per_cluster;

    fprintf(stddebug, "Data start: %d\n", data_start);
    int cluster_index = 1; // we print the first cluster outside the loop

    fprintf(stddebug, "Reading cluster %d\n", cluster);
    if (!read_sectors_lba(fs->device,  ((cluster - 2) * sectors_per_cluster) + data_start, sectors_per_cluster, buffer)) {
        fprintf(stderr, "Read failure\n");
        return FAILURE;
    }

    for(;;) {
        fprintf(stddebug, "Reading cluster %d\n", cluster);
        cluster = fat12_decode_fat_entry(cluster, fs);
        if(cluster_is_end_of_chain(cluster, fs)) break; // we've read the last cluster already

        if (!read_sectors_lba(fs->device,  ((cluster - 2) * sectors_per_cluster) + data_start, sectors_per_cluster, buffer + (BYTES_PER_SECTOR * cluster_index))) {
            fprintf(stderr, "Read failure\n");
            return FAILURE;
        }

        cluster_index++;
    }

    return SUCCESS;
}

/**
 * Private functions
 */

/** Read the whole FAT into a buffer */
static bool fat12_read_fat(DiskDevice* device, BiosParameterBlock *bpb, uint8_t *fat) {
    fprintf(stddebug, "Reading FAT\n");
    return read_sectors_lba(device, bpb->num_reserved_sectors, bpb->sectors_per_fat, fat);
}

/** Return the location of the next cluster, if any, given a cluster number */
static uint32_t fat12_decode_fat_entry(uint32_t cluster_num, FileSystem *fs) {
    uint8_t* fat = instance_data(fs)->fat;
    char fat_type = instance_data(fs)->fat_type;
    if (fat_type != FAT12) {
        // This is the easy case, for FAT16 and FAT32, as they are multiples of 1 byte
        return *(fat + cluster_num);
    }

    // For FAT12 we have to do a bit of a horrible calculation
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

/** Return the location of the next cluster, if any, given a cluster number */
static uint32_t fat32_decode_fat_entry(uint32_t cluster_num, const uint8_t *fat) {
    return *(fat + cluster_num);
}

static FatInstanceData * instance_data(FileSystem *fs) {
    return ((FatInstanceData *) fs->instance_data);
}

static int root_dir_start(FileSystem *fs) {
    BiosParameterBlock *bpb = ((FatInstanceData *) fs->instance_data)->bpb;
    return bpb->num_reserved_sectors + (bpb->sectors_per_fat * bpb->num_fats);
}

static int first_data_sector(FileSystem *fs) {
    BiosParameterBlock *bpb = ((FatInstanceData *) fs->instance_data)->bpb;
    return bpb->num_reserved_sectors + (bpb->sectors_per_fat * bpb->num_fats) + (bpb->max_root_directory_entries * DIR_ENTRY_SIZE / BYTES_PER_SECTOR);
}

static bool cluster_is_end_of_chain(uint32_t cluster, FileSystem *fs) {
    char fat_type = instance_data(fs)->fat_type;
    if (fat_type == FAT12) {
        return (bool) (cluster >= 0xFF8);
    } else if (fat_type == FAT16) {
        return (bool) (cluster >= 0xFFF8);
    } else {
        return (bool) ((cluster & 0x0FFFFFFF) >= 0x0FFFFFF8);
    }
}
