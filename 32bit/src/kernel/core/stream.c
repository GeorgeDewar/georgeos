#include "system.h"

// typedef struct {
//     uint8_t fp_type     : 4;
//     uint8_t can_read    : 1;
//     uint8_t can_write   : 1;
//     struct StreamDevice stream_device;
// } FilePointer;
// enum FPType {
//     RESERVED = 0,
//     STREAM,
//     FILE
// };

// /** array of currently open files */
// FilePointer files[16];

static void vfprintf(int16_t fp, char* string, va_list argp);

void stream_init() {
    
}

uint8_t get_stream_device(int16_t fp, struct StreamDevice* sd_out) {
    if (fp == stdout || fp == stderr) {
        *sd_out = sd_screen_console;
    } else if (fp == stddebug) {
        *sd_out = sd_com1;
    } else {
        die("Unknown stream device");
    }
    return 1;
}

bool write(int16_t fp, char* buffer, int len) {
    struct StreamDevice sd = {};
    get_stream_device(fp, &sd);
    sd.write(buffer, len);
    return SUCCESS;
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
    struct StreamDevice sd = {};
    get_stream_device(fp, &sd);

    while(*string != 0) {
        if (*string == '%') {
            string++;
            if(*string == '%'){ // %% escapes %
                sd.write(string, 1);
            } else if (*string == 's') {
                char* str = (char*) va_arg(argp, int);
                fprintf(fp, str);
            } else if (*string == 'c') {
                char c = va_arg(argp, int);
                sd.write(&c, 1);
            } else if (*string == 'd') {
                int number = va_arg(argp, int);
                char num_string[16];
                intToString(number, num_string);
                fprintf(fp, num_string);
            }
        } else {
            sd.write(string, 1);
        }
        string++;
    }
}
