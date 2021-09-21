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

void stream_init() {
    
}

uint8_t get_stream_device(int16_t fp, struct StreamDevice* sd_out) {
    if (fp == stdout) {
        *sd_out = sd_screen_console;
    } else if (fp == stddebug) {
        *sd_out = sd_com1;
    } else {
        die("Unknown stream device");
    }
    return 1;
}

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
    struct StreamDevice sd = {};
    get_stream_device(fp, &sd);

    while(*string != 0) {
        if (*string == '%') {
            string++;
            if(*string == '%'){ // %% escapes %
                sd.print_char(*string);
            } else if (*string == 's') {
                char* str = va_arg(argp, int);
                fprintf(fp, str);
            } else if (*string == 'd') {
                int number = va_arg(argp, int);
                char num_string[16];
                intToString(number, num_string);
                fprintf(fp, num_string);
            }
        } else {
            sd.print_char(*string);
        }
        string++;
    }

}

void fprintf(int16_t fp, char* string) {
    struct StreamDevice sd = {};
    get_stream_device(fp, &sd);

    int i = 0;
    while(string[i] != 0) {
        sd.print_char(string[i++]);
    }
    // void (*print_fn)(char character);
    // FilePointer fp_obj = files[fp];
    // if (fp_obj.fp_type == STREAM) {
    //     print_fn = fp_obj.stream_device.print_char;
    // }

}
