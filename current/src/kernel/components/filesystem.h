// FAT12

#define BYTES_PER_SECTOR        512
#define NUM_RESERVED_SECTORS    1
#define NUMBER_OF_FATS          2
#define SECTORS_PER_FAT         9
#define ROOT_DIR_ENTRIES        224
#define DIR_ENTRY_SIZE          32

#define FAT_0_START             NUM_RESERVED_SECTORS
#define ROOT_DIR_START          NUM_RESERVED_SECTORS + (NUMBER_OF_FATS * SECTORS_PER_FAT)
#define ROOT_DIR_LENGTH         (ROOT_DIR_ENTRIES * DIR_ENTRY_SIZE) / BYTES_PER_SECTOR

struct DirectoryEntry {
    char name[8];                                                                               // 00
    char extension[3];                                                                          // 08

    // Bit field
    unsigned char readOnly       : 1;                                                           // 11
    unsigned char hidden         : 1;
    unsigned char system         : 1;
    unsigned char volumeLabel    : 1;
    unsigned char subdirectory   : 1;
    unsigned char archive        : 1;
    unsigned char device         : 1;
    unsigned char                : 1;

    char unused_0C; // 0x0C; CP/M-86 and DOS Plus store user attributes here                    // 12
    char unused_0D; // 0x0D; Various purposes under some DOS versions                           // 13
    int unused_0E;  // 0x0E; Various purposes under some DOS versions                           // 14
    int unused_10;  // 0x10; Various purposes under some DOS versions                           // 16
    int unused_12;                                                                              // 18
    int unused_14;                                                                              // 20
    int lastModifiedTime;                                                                       // 22
    int lastModifiedDate;                                                                       // 24

    unsigned int startOfFile;                                                                   // 26
    unsigned long fileSize;                                                                     // 28-32
};

void listFiles(char* buffer);
char readRootDirectory(char* buffer);
char getFileName(struct DirectoryEntry e, char* buffer);

char loadFAT();
long loadFile(struct DirectoryEntry dir[], int directoryLength, char* filename, int segment, char* buffer);
unsigned int findFile(struct DirectoryEntry dir[], int directoryLength, char* filename);
static unsigned int getFATEntry(unsigned int cluster_num);
