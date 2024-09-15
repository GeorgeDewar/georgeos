#include "system.h"

#define HEAP_START  0xA00000    // 16 MB
void *free_memory_start = (void *) HEAP_START;

/** A super-simple 'watermark' allocator which cannot free memory */
void *malloc(size_t size) {
    void *block_to_allocate = free_memory_start;
    free_memory_start += size;
    return block_to_allocate;
}

void *memalign(size_t alignment, size_t size) {
    // Allocate enough to get the required aligned block, and return it
    uint32_t diff = ((uint8_t) free_memory_start) & (alignment - 1); // this is how much has been allocated above the last alignment boundary
    uint32_t waste = alignment - diff;
    void *allocation = malloc(size + waste);
    return allocation + waste;
}

/** We can't do this yet, so we just pretend */
void free(void *ptr) {
    // Do nothing
}

void memset(void* dest, uint8_t value, uint32_t length) {
    for(uint32_t i=0; i<length; i++) {
        ((uint8_t *) dest)[i] = value;
    }
}

void memcpy(void* source, void* dest, uint32_t length) {
    for(uint32_t i=0; i<length; i++) {
        ((uint8_t *) dest)[i] = ((uint8_t *) source)[i];
    }
}

/**
 * Print out the specified number of bytes from the buffer, one byte at a time, 8 per line
 */
void dump_mem8(int16_t fp, char *prefix, char *buffer, int bytes) {
    int lines = bytes / 8;
    fprintf(fp, "Dumping %d bytes from 0x%08x\n", bytes, buffer);
    for(int line=0; line<lines; line++) {
        char *dwords = buffer + (line * 8);
        fprintf(fp, "%s %02x %02x %02x %02x %02x %02x %02x %02x\n", prefix, 
            dwords[0] & 0xff, dwords[1] & 0xff, dwords[2] & 0xff, (uint32_t) dwords[3] & 0xff, 
            (uint32_t) dwords[4] & 0xff, (uint32_t) dwords[5] & 0xff,  (uint32_t) dwords[6] & 0xff,  (uint32_t) dwords[7] & 0xff);
    }
}

/**
 * Print out the specified number of bytes from the buffer, one dword at a time, 1 per line
 */
void dump_mem32(int16_t fp, char *prefix, uint32_t *buffer, int bytes) {
    int lines = bytes / 4;
    fprintf(fp, "Dumping %d bytes from 0x%08x\n", bytes, buffer);
    for(int line=0; line<lines; line++) {
        uint32_t *dwords = buffer + (line);
        fprintf(fp, "%s %08x: %08x\n", prefix, dwords, 
            dwords[0]);
    }
}