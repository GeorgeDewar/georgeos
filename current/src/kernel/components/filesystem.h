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
    unsigned char attributes;

    char unused_0C; // 0x0C; CP/M-86 and DOS Plus store user attributes here                    // 12
    char unused_0D; // 0x0D; Various purposes under some DOS versions                           // 13
    int unused_0E;  // 0x0E; Various purposes under some DOS versions                           // 14
    int unused_10;  // 0x10; Various purposes under some DOS versions                           // 16
    int unused_12;                                                                              // 18
    int unused_14;                                                                              // 20
    int lastModifiedTime;                                                                       // 22
    int lastModifiedDate;                                                                       // 24

    unsigned int startOfFile;                                                                   // 26
    unsigned int fileSize1;                                                                     // 28-32
    unsigned int fileSize2;
};

void listFiles(char* buffer);
char readRootDirectory(char* buffer);
char getFileName(struct DirectoryEntry e, char* buffer);

char loadFAT();
int loadFile(struct DirectoryEntry dir[], int directoryLength, char* filename, char* buffer);
unsigned int findFile(struct DirectoryEntry dir[], int directoryLength, char* filename);
static unsigned int getFATEntry(unsigned int cluster_num);
