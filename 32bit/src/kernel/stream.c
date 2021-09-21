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

void printf(char* string) {
    fprintf(stdout, string);
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
