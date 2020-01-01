#include "filesystem.h";

#include "../bios/disk.h";

char readRootDirectory(char* buffer) {
    return readSectorLBA(ROOT_DIR_START, buffer);
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
