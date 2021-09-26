#include "system.h"

DEFN_SYSCALL1(print, 0, const char *);
DEFN_SYSCALL3(get_string, 1, char *, int, int);
DEFN_SYSCALL3(list_dir, 2, char *, char *, int *);
DEFN_SYSCALL1(exec, 3, const char *);
// Todo: Change to read and write, add exit (for now, works by restarting shell like in DOS), add exec (CreateProcess)
// getcwd, chdir, listdir

void intToString(int number, char* string) {
    int index = 0;
    char buffer[16] = "";
    int len;

    do {
        int thisPlace = number % 10;
        buffer[index++] = '0' + thisPlace;
        number = number / 10;
    } while(number > 0);
    
    // Reverse it
    len = index;
    index = 0;
    for(index; index < len; index++) {
        string[index] = buffer[len-index-1];
    }
    string[index] = 0;
}

void printf(char* string, ...) {
    va_list argp;
    va_start(argp, string);

    vfprintf(stdout, string, argp);
    va_end(argp);
}

void vfprintf(int16_t fp, char* string, va_list argp) {
    char buffer[128];
    vsprintf(buffer, string, argp);
    sys_print(buffer);
}

void vsprintf(char* buffer, char* string, va_list argp) {
    while(*string != 0) {
        if (*string == '%') {
            string++;
            if(*string == '%'){ // %% escapes %
                *buffer++ = *string;
            } else if (*string == 's') {
                char* str = va_arg(argp, int);
                strcpy(str, buffer);
                buffer += strlen(str); // erase null byte
            } else if (*string == 'd') {
                int number = va_arg(argp, int);
                char num_string[16];
                intToString(number, num_string);
                strcpy(num_string, buffer);
                buffer += strlen(num_string); // erase null byte
            }
        } else {
            *buffer++ = *string;
        }
        string++;
    }
    *buffer = 0;
}

// This is the memory space for our single userland process
#define HEAP_START  0xB00000    // 32 MB
void *free_memory_start = HEAP_START;

/** A super-simple 'watermark' allocator which cannot free memory */
void *malloc(uint32_t size) {
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

/** compare two strings, stopping after length_to_compare characters */
char strcmp_wl(char *string1, char *string2, uint32_t length_to_compare) {
    uint32_t i = 0;

    while(i < length_to_compare) {
        if(string1[i] == 0 && string2[i] == 0) {
            return 1; // end of both strings
        }
        if(string1[i] != string2[i]) {
            return 0; // strings are not equal
        }
        i++;
    }

    return 1;
}

/** compare two strings */
char strcmp(char *string1, char *string2) {
    return strcmp_wl(string1, string2, UINT32_T_MAX);
}

/** copy a string from src to dest, stopping at the NULL byte */
void strcpy(char *src, char *dest) {
    while(*src != 0) {
        *dest++ = *src++;
    }
    *dest = 0;
}

int strlen(char *str) {
    int len = 0;
    while(*str++ != 0) {
        len++;
    }
    return len;
}
