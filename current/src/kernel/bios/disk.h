struct DirectoryEntry {
    char name[8];
    char extension[3];

    // Bit field
    unsigned int readOnly       : 1;
    unsigned int hidden         : 1;
    unsigned int system         : 1;
    unsigned int volumeLabel    : 1;
    unsigned int subdirectory   : 1;
    unsigned int archive        : 1;
    unsigned int device         : 1;
    unsigned int                : 1;

    char unused_0C; // 0x0C; CP/M-86 and DOS Plus store user attributes here
    char unused_0D; // 0x0D; Various purposes under some DOS versions
    int unused_0E;  // 0x0E; Various purposes under some DOS versions
    int unused_10;  // 0x10; Various purposes under some DOS versions
    int unused_12;
    int unused_14;
    int lastModifiedTime;
    int lastModifiedDate;

    unsigned int startOfFile;
    unsigned int fileSize; // meant to be 4 bytes so something else is probably wrong
};

char readRoot(char* buffer);
