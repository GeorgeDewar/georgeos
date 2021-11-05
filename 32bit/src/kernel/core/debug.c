#include "system.h"

void print_mem_info() {
    void* p = NULL;
    extern void *free_memory_start;
    fprintf(stdout, "Stack: %x, KHeap: %x\n", (void*)&p, free_memory_start);
}
