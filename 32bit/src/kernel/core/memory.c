#include "system.h"

#define HEAP_START  0xA00000    // 16 MB
void *free_memory_start = HEAP_START;

/** A super-simple 'watermark' allocator which cannot free memory */
void *malloc(size_t size) {
    void *block_to_allocate = free_memory_start;
    free_memory_start += size;
    return block_to_allocate;
}

/** We can't do this yet, so we just pretend */
void free(void *ptr) {
    // Do nothing
}

void memset(uint8_t* source, uint8_t value, uint32_t length) {
    for(uint32_t i=0; i<length; i++) {
        source[i] = value;
    }
}

void memcpy(uint8_t* source, uint8_t* dest, uint32_t length) {
    for(uint32_t i=0; i<length; i++) {
        dest[i] = source[i];
    }
}
