#include "filesystem.h";

#include "../bios/disk.h";

char readRootDirectory(char* buffer) {
    return readSectorLBA(ROOT_DIR_START, buffer);
}
