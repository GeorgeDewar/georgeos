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

char readRoot(char* buffer);
