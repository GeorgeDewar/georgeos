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
