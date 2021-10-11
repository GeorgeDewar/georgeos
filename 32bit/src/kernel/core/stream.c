#include "system.h"

static void vfprintf(int16_t fp, char* string, va_list argp);

/** Read from a file into the specified buffer, up to *len* bytes */
bool read(int16_t fp, char* buffer, int len) {
    FileHandle handle = open_files[fp];
    if (handle.type == NULL) {
        // The handle does not exist
        return FAILURE;
    } else if(handle.type == STREAM) {
        handle.stream_device.read(buffer, len, true);
        return SUCCESS;
    } else {
        // Unsupported handle type
        return FAILURE;
    }
}

/** Write to a file from the specified buffer, *len* bytes */
bool write(int16_t fp, char* buffer, int len) {
    FileHandle handle = open_files[fp];
    if (handle.type == NULL) {
        // The handle does not exist
        return FAILURE;
    } else if(handle.type == STREAM) {
        handle.stream_device.write(buffer, len);
        return SUCCESS;
    } else {
        // Unsupported handle type
        return FAILURE;
    }
}

static void intToString(int number, char* string) {
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
    for(; index < len; index++) {
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

void fprintf(int16_t fp, char* string, ...) {
    va_list argp;
    va_start(argp, string);

    vfprintf(fp, string, argp);
    va_end(argp);
}

static void vfprintf(int16_t fp, char* string, va_list argp) {
    while(*string != 0) {
        if (*string == '%') {
            string++;
            if(*string == '%'){ // %% escapes %
                write(fp, string, 1);
            } else if (*string == 's') {
                char* str = (char*) va_arg(argp, int);
                fprintf(fp, str);
            } else if (*string == 'c') {
                char c = va_arg(argp, int);
                write(fp, &c, 1);
            } else if (*string == 'd') {
                int number = va_arg(argp, int);
                char num_string[16];
                intToString(number, num_string);
                fprintf(fp, num_string);
            }
        } else {
            write(fp, string, 1);
        }
        string++;
    }
}
