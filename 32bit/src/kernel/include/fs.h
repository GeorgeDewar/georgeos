typedef struct {
    uint16_t bytes_per_sector;
    uint8_t sectors_per_cluster;
    uint16_t num_reserved_sectors;
    uint8_t num_fats;
    uint16_t max_root_directory_entries;
    uint16_t num_sectors;
    uint8_t media_descriptor;
    uint16_t sectors_per_fat;
    uint16_t sectors_per_track;
    uint16_t num_heads;
    uint32_t num_hidden_sectors;
    uint32_t total_logical_sectors_large;
} __attribute__((packed)) Dos331BiosParameterBlock;

typedef struct {
    Dos331BiosParameterBlock bpb;
    uint8_t physical_drive_no;
    uint8_t reserved;
    uint8_t extended_boot_signature;
    uint32_t volume_id;
    char partition_volume_label[11];
    char file_system_type[8];
} __attribute__((packed)) ExtendedBiosParameterBlock;

typedef struct {
    Dos331BiosParameterBlock bpb;
    uint32_t sectors_per_fat;
    uint16_t drive_description_and_mirroring;
    uint16_t version;
    uint32_t root_dir_start;
    uint16_t fs_info_sector;
    uint16_t backup_boot_sector_start;
    uint8_t reserved[12];
    uint8_t physical_drive_no;
    uint8_t reserved1;
    uint8_t extended_boot_signature;
    uint32_t volume_id;
    char partition_volume_label[11];
    char file_system_type[8];
} __attribute__((packed)) Fat32ExtendedBiosParameterBlock;

//typedef struct {
//    Dos331BiosParameterBlock bpb;
//    union {
//        ExtendedBiosParameterBlock ebpb;
//        Fat32ExtendedBiosParameterBlock fat32_ebpb;
//    };
//} FatUnionBPB;

typedef struct {
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
} FatVolumeInfo;
