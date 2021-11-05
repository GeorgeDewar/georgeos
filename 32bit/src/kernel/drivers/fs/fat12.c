#include "system.h"
#include "fs.h"

#define BYTES_PER_SECTOR        512
#define DIR_ENTRY_SIZE          32
#define DIR_ENTRIES_PER_SECTOR  (BYTES_PER_SECTOR / DIR_ENTRY_SIZE)

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
    char fat_type;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint32_t sectors_per_fat;
    uint16_t max_root_directory_entries; // FAT12/16 only
    uint32_t total_sectors;
    uint32_t root_dir_start;
    uint32_t volume_id;
    char partition_volume_label[11];
    char file_system_type[8];
} FatInstanceData;
enum FatType {
    FAT12 = 0,
    FAT16,
    FAT32,
};
char *FatTypeName[] = {
    "FAT12",
    "FAT16",
    "FAT32",
};

/**
 * Header section
 */
bool fat12_init(DiskDevice* device, FileSystem* filesystem_out);
bool fat12_list_root(FileSystem* fs, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
bool fat12_list_dir(FileSystem* fs, uint32_t cluster, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);
bool fat12_read_file(FileSystem * fs, uint32_t cluster, void* buffer, uint32_t* clusters_read);
static bool fat12_read_fat(DiskDevice* device, FatInstanceData *instance_data, uint8_t *fat);
static uint32_t fat12_decode_fat_entry(uint32_t cluster_num, FileSystem *fs);
static FatInstanceData * get_instance_data(FileSystem *fs);
static unsigned int first_data_sector(FileSystem *fs);
static bool cluster_is_end_of_chain(uint32_t cluster, FileSystem *fs);
static bool fat_parse_dir_entries(FileSystem* fs, void *buffer_in, uint16_t max_entries, DirEntry* dir_entry_list_out, uint16_t* num_entries_out);

FileSystemDriver fs_fat12 = {
    .init = &fat12_init,
    .list_root = &fat12_list_root,
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
    Dos331BiosParameterBlock *bpb = (Dos331BiosParameterBlock *) (buffer + 11);

    char fat_type;
    if (strcmp_wl(((ExtendedBiosParameterBlock *) bpb)->file_system_type, FatTypeName[FAT12], 5) > 0) {
        fat_type = FAT12;
    } else if (strcmp_wl(((ExtendedBiosParameterBlock *) bpb)->file_system_type, FatTypeName[FAT16], 5) > 0) {
        fat_type = FAT16;
    } else if (strcmp_wl(((Fat32ExtendedBiosParameterBlock *) bpb)->file_system_type, FatTypeName[FAT32], 5) > 0) {
        fat_type = FAT32;
    } else {
        fprintf(stddebug, "Not a FAT volume\n");
        return FAILURE;
    }

    // Allocate memory for instance data structure
    FatInstanceData *instance_data = malloc(sizeof(FatInstanceData));

    // Parse BPB/EBPB info into a consistent structure. Some things are common between all FAT versions, while others
    // are located in different places for FAT32 vs FAT12/16.
    instance_data->sectors_per_cluster = bpb->sectors_per_cluster;
    instance_data->num_reserved_sectors = bpb->num_reserved_sectors;
    instance_data->num_fats = bpb->num_fats;
    instance_data->max_root_directory_entries = bpb->max_root_directory_entries;
    if (bpb->num_sectors > 0) {
        instance_data->total_sectors = bpb->num_sectors;
    } else if (bpb->total_logical_sectors_large > 0) { // only applicable if the above is zero
        instance_data->total_sectors = bpb->total_logical_sectors_large;
    } else {
        // Could get it from the partition record but we will probably never see this condition
        fprintf(stderr, "FAT BPB does not specify number of sectors\n");
        return FAILURE;
    }

    if (fat_type == FAT32) {
        Fat32ExtendedBiosParameterBlock *ebpb = (Fat32ExtendedBiosParameterBlock *) bpb;
        instance_data->sectors_per_fat = ebpb->sectors_per_fat;
        instance_data->root_dir_start = ebpb->root_dir_start;
        instance_data->volume_id = ebpb->volume_id;
        memcpy(ebpb->partition_volume_label, instance_data->partition_volume_label, sizeof(ebpb->partition_volume_label));
    } else {
        ExtendedBiosParameterBlock *ebpb = (ExtendedBiosParameterBlock *) bpb;
        instance_data->sectors_per_fat = bpb->sectors_per_fat;
        // This value is not specified in the BPB, as it can be calculated based on other values
        instance_data->root_dir_start = instance_data->num_reserved_sectors + (instance_data->sectors_per_fat * instance_data->num_fats);
        instance_data->volume_id = ebpb->volume_id;
        memcpy(ebpb->partition_volume_label, instance_data->partition_volume_label, sizeof(ebpb->partition_volume_label));
    }

    // Allocate buffer to store File Allocation Table
    uint8_t *fat = malloc(instance_data->sectors_per_fat * BYTES_PER_SECTOR);
    instance_data->fat = fat;
    instance_data->fat_type = fat_type;

    // Populate the output fields
    filesystem_out->driver = &fs_fat12;
    filesystem_out->device = device;
    filesystem_out->instance_data = instance_data;
    filesystem_out->case_sensitive = false;

    // Print debug info
    fprintf(stddebug, "Initialising %s volume\n", FatTypeName[fat_type]);
    fprintf(stddebug, "  Reserved sectors: %d\n",instance_data->num_reserved_sectors);
    fprintf(stddebug, "  Sectors per FAT: %d\n", instance_data->sectors_per_fat);
    fprintf(stddebug, "  Number of FATs: %d\n", instance_data->num_fats);
    fprintf(stddebug, "  Root dir entries: %d\n", instance_data->max_root_directory_entries);

    // Print user-readable message
    char label[12];
    memcpy(instance_data->partition_volume_label, label, 11);
    label[11] = 0;
    printf("Mounting %s filesystem with label %s\n", FatTypeName[fat_type], label);

    // Read the FAT
    if (fat12_read_fat(device, instance_data, fat) == FAILURE) return FAILURE;
    return SUCCESS;
}

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool fat12_list_root(FileSystem* fs, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    if (get_instance_data(fs)->fat_type == FAT32) {
        // The root directory is a cluster chain, just like a normal file
        return fat12_list_dir(fs, get_instance_data(fs)->root_dir_start, dir_entry_list_out, num_entries_out);
    }

    int sectors_per_dir = get_instance_data(fs)->max_root_directory_entries * DIR_ENTRY_SIZE / BYTES_PER_SECTOR;
    int total_entries = 0;
    for(int i=0; i<sectors_per_dir; i++) {
        uint16_t entries_this_sector = 0;
        uint8_t buffer[BYTES_PER_SECTOR];
        if (read_sectors_lba(fs->device, get_instance_data(fs)->root_dir_start + i, 1, buffer) == FAILURE) {
            return FAILURE;
        }
        fat_parse_dir_entries(fs, buffer, DIR_ENTRIES_PER_SECTOR, dir_entry_list_out, &entries_this_sector);
        total_entries += entries_this_sector;
        if (entries_this_sector < DIR_ENTRIES_PER_SECTOR) break;
    }
    *num_entries_out = total_entries;
    return SUCCESS;
}

/** List the files in a directory into dir_entry_list_out, and set num_entries_out */
bool fat12_list_dir(FileSystem* fs, uint32_t cluster, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    // Read the directory like a file
    uint32_t clusters_read;
    uint8_t buffer[DIR_ENTRY_SIZE * 128]; // Will overflow if more than 128 files in dir
    if (fat12_read_file(fs, cluster, buffer, &clusters_read) == FAILURE) return FAILURE;

    return fat_parse_dir_entries(fs, buffer, clusters_read * get_instance_data(fs)->sectors_per_cluster * DIR_ENTRIES_PER_SECTOR, dir_entry_list_out, num_entries_out);
}

/**
 * Parse up to max_entries directory entries from the supplied buffer, into standard directory entries in the output
 * buffer, dir_entry_list_out. num_entries_out is set the the number of entries actually found.
 */
static bool fat_parse_dir_entries(FileSystem* fs, void *buffer_in, uint16_t max_entries, DirEntry* dir_entry_list_out, uint16_t* num_entries_out) {
    fprintf(stddebug, "Parsing up to %d dirents\n", max_entries);

    // Create a buffer large enough for one sector worth of directory entries
    FatDirectoryEntry* dir_entry_buffer = (FatDirectoryEntry*) buffer_in;

    for (int i=0; i < max_entries; i++) {
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
        uint16_t start_of_file_high = get_instance_data(fs)->fat_type == FAT32 ? fat12entry.fat32_start_of_file_high : 0;
        dir_entry_list_out[i].location_on_disk = fat12entry.start_of_file_low + (start_of_file_high << 16);
    }

    *num_entries_out = max_entries;
    return SUCCESS;
}

/** Read an entire file into the supplied buffer, and set length_out */
bool fat12_read_file(FileSystem * fs, uint32_t cluster, void* buffer, uint32_t* clusters_read) {
    unsigned int data_start = first_data_sector(fs);
    int sectors_per_cluster = get_instance_data(fs)->sectors_per_cluster;

    fprintf(stddebug, "Data start: %d\n", data_start);
    int cluster_index = 0;

    for(;;) {
        fprintf(stddebug, "Reading cluster %d\n", cluster);

        if (!read_sectors_lba(fs->device,  ((cluster - 2) * sectors_per_cluster) + data_start, sectors_per_cluster, buffer + (BYTES_PER_SECTOR * cluster_index))) {
            fprintf(stderr, "Read failure\n");
            return FAILURE;
        }

        cluster = fat12_decode_fat_entry(cluster, fs);
        if (cluster_is_end_of_chain(cluster, fs)) break; // we've read the last cluster already
        if (cluster < 2) {
            fprintf(stderr, "Invalid cluster value: %d\n", cluster);
            return FAILURE;
        }

        cluster_index++;
    }

    *clusters_read = cluster_index + 1;
    return SUCCESS;
}

/**
 * Private functions
 */

/** Read the whole FAT into a buffer */
static bool fat12_read_fat(DiskDevice* device, FatInstanceData *instance_data, uint8_t *fat) {
    fprintf(stddebug, "Reading FAT\n");
    return read_sectors_lba(device, instance_data->num_reserved_sectors, instance_data->sectors_per_fat, fat);
}

/** Return the location of the next cluster, if any, given a cluster number */
static uint32_t fat12_decode_fat_entry(uint32_t cluster_num, FileSystem *fs) {
    uint8_t* fat = get_instance_data(fs)->fat;
    char fat_type = get_instance_data(fs)->fat_type;
    if (fat_type == FAT32) {
        // This is the easy case, for FAT16 and FAT32, as they are multiples of 1 byte
        return *(((uint32_t*) fat) + cluster_num);
    } else if (fat_type == FAT16) {
        return *(((uint16_t*) fat) + cluster_num);
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

static FatInstanceData * get_instance_data(FileSystem *fs) {
    return ((FatInstanceData *) fs->instance_data);
}

static unsigned int first_data_sector(FileSystem *fs) {
    return get_instance_data(fs)->num_reserved_sectors +
           (get_instance_data(fs)->sectors_per_fat * get_instance_data(fs)->num_fats) +
           (get_instance_data(fs)->max_root_directory_entries * DIR_ENTRY_SIZE / BYTES_PER_SECTOR);
}

static bool cluster_is_end_of_chain(uint32_t cluster, FileSystem *fs) {
    char fat_type = get_instance_data(fs)->fat_type;
    if (fat_type == FAT12) {
        return (bool) (cluster >= 0xFF8);
    } else if (fat_type == FAT16) {
        return (bool) (cluster >= 0xFFF8);
    } else {
        return (bool) ((cluster & 0x0FFFFFFF) >= 0x0FFFFFF8);
    }
}
