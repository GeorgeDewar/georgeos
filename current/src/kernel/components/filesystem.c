#include "filesystem.h";

#include "../bios/disk.h";
#include "../util/string.h";

char FAT[SECTORS_PER_FAT * BYTES_PER_SECTOR];

char readRootDirectory(char* buffer) {
    return readSectorsLBA(ROOT_DIR_START, 1, buffer);
}

char getFileName(struct DirectoryEntry e, char* buffer) {
    char* filename = e.name;
    char* extension = e.extension;

    unsigned char index1 = 0, bufferIndex = 0;

    while(filename[index1] != ' ' && filename[index1] != 0) {
        buffer[bufferIndex++] = filename[index1++];
    }

    buffer[bufferIndex++] = '.';

    index1 = 0;
    while(extension[index1] != ' ' && extension[index1] != 0) {
        buffer[bufferIndex++] = extension[index1++];
    }

    return bufferIndex; // length of filename
}

char loadFAT() {
    return readSectorsLBA(FAT_0_START, SECTORS_PER_FAT, FAT);
}

char loadFile(char* filename, char* buffer) {
    return -1;
}

// Private functions

/*
 * Find a file and return its first cluster number.
 * 
 * Returns -1 if the file cannot be found
 */
unsigned int findFile(struct DirectoryEntry dir[], int directoryLength, char* filename) {
    int i;
    for(i=0; i<directoryLength; i++) {
        char thisFilename[16];
        getFileName(dir[i], thisFilename);
        if(strcmp(filename, thisFilename)) {
            return dir[i].startOfFile;
        }
    }
    return -1;
}

static unsigned int getFATEntry(unsigned int cluster_num) {
    unsigned int offset;
    unsigned char b1, b2;
    
    /* this involves some really ugly bit shifting.  This probably
       only works on a little-endian machine. */
    offset = NUM_RESERVED_SECTORS * BYTES_PER_SECTOR + (3 * (cluster_num / 2));
    if (cluster_num % 2 == 0) {
        b1 = *(FAT + offset);
        b2 = *(FAT + offset + 1);
        return ((0x0f & b2) << 8) | b1;
    }
    else {
        b1 = *(FAT + offset + 1);
        b2 = *(FAT + offset + 2);
        return b2 << 4 | ((0xf0 & b1) >> 4);
    }
}
