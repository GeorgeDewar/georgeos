#include "filesystem.h";

#include "../bios/disk.h";
#include "../bios/keyboard.h";
#include "../util/string.h";
#include "../components/console.h";

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

    buffer[bufferIndex] = 0;

    return bufferIndex; // length of filename
}

char loadFAT() {
    return readSectorsLBA(FAT_0_START, SECTORS_PER_FAT, FAT);
}

char loadFile(struct DirectoryEntry dir[], int directoryLength, char* filename, char* buffer) {
    int clusterIndex = 0;
    int readResult;
    unsigned int cluster;
    
    println("Start");

    // Find the first cluster number from the directory entry
    cluster = findFile(dir, directoryLength, filename);
    if(cluster == -1) return -1;

    print("Found first cluster: ");
    printInt(cluster);
    println("");

    readResult = readSectorsLBA(cluster + 31, 1, buffer);
    if(readResult != 0) return readResult; // Error
    println("Loaded first cluster");

    while(1==1) {
        cluster = getFATEntry(cluster);
        if(cluster == 0xFFF) break; // we've read the last cluster already
        println("Found cluster: ");
        printInt(cluster);
        println("");
        getChar();

        readResult = readSectorsLBA(cluster + 31, 1, buffer + (BYTES_PER_SECTOR * clusterIndex));
        if(readResult != 0) return readResult; // Error
        println("Loaded cluster");

        clusterIndex++;
    }

    return 0;
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
        println(thisFilename);
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
    offset = (3 * (cluster_num / 2));

    print("Offset: ");
    printInt(offset);
    println("");

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
