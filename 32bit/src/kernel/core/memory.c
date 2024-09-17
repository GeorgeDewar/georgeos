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
    //printf("Allocation %d bytes with %d alignment. Current start is %08x\n", size, alignment, free_memory_start);
    uint32_t diff = ((uint32_t) free_memory_start) & (alignment - 1); // this is how much has been allocated above the last alignment boundary
    if (diff == 0) return malloc(size);

    uint32_t waste = alignment - diff;
    void *allocation = malloc(size + waste);
    //printf("Diff: %d, Waste: %d, Allocation @ %08x, Returning %08x\n", diff, waste, allocation, allocation + waste);
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
    int bytes_per_line = 8;
    fprintf(fp, "Dumping %d bytes from 0x%08x", bytes, buffer);
    
    for(int i=0; i<bytes; i++) {
        char byte = buffer[i];
        if (i % bytes_per_line == 0) {
            fprintf(fp, "\n%s", prefix);
        }
        fprintf(fp, "%02x ", byte & 0xFF);
    }
    fprintf(fp, "\n");
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