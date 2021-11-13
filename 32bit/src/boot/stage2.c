#include "boot.h"

void print_string(char *str);
void print_char(char c);

void main() {
    print_string("Stage2!\r\n");
    vbe_info_block *infoblock = (vbe_info_block *) 0x9000;
    uint16_t result = get_mode_list((uint16_t) infoblock);
    if (result == 0x004f) {
        print_string("Success\r\n");
    } else {
        print_string("VBE Error\r\n");
        for(;;);
    }

    set_ds(infoblock->OemStringPtr[0])
    char *oem
    set_ds(0);

    for(;;);
}

void set_ds(uint16_t segment) {
    asm("mov %0, ds" :: "a" (segment));
}

void print_string(char *str) {
    while(*str != 0) {
        print_char(*str++);
    }
}

void print_char(char c) {
    asm(
        "mov %0, %%al;"
        "mov $0x0E, %%ah;"
        "int $0x10;"
        : // no output
        : "a" (c) // %0 = c
    );
}

uint16_t get_mode_list(uint16_t dest_addr) {
    uint16_t output = 0;
    asm(
    "mov %0, %%di;"
    "mov $0x4f00, %%ax;"
    "int $0x10;"
    : "=a" (output) // ax register
    : "r" (dest_addr)
    );
    return output;
}
