#include "boot.h"

void print_string(char *str);
void print_char(char c);

void main() {
    print_string("Stage2!\r\n");
    struct vbe_info_block *infoblock = (struct vbe_info_block *) 0x9000;
    infoblock->VbeSignature[0] = 'V';
    infoblock->VbeSignature[1] = 'B';
    infoblock->VbeSignature[2] = 'E';
    infoblock->VbeSignature[3] = '2';

    uint16_t result;
    char* edid_buffer = (char *) 0x9200;
    result = read_edid(edid_buffer);
    if (result == 0x004f) {
        print_string("EDID Success\r\n");
    }
    for(;;);

    result = get_mode_list((uint16_t) infoblock);
    if (result == 0x004f) {
        print_string("Success\r\n");
    } else {
        print_string("VBE Error\r\n");
        for(;;);
    }

    char* vendor_name = (char *) (infoblock->OemVendorNamePtr[1] * 0x10 + infoblock->OemVendorNamePtr[0]);
    if (vendor_name > 0xFFFF) {
        print_string("Vendor name outside segment\r\n");
    } else {
        print_string(vendor_name);
    }

    uint16_t *video_modes = (uint16_t *) (infoblock->VideoModePtr[1] * 0x10 + infoblock->VideoModePtr[0]);
    while(1) {
        if (*video_modes == 0xFFFF) break;

        char buffer[5];
        int_to_string(*video_modes++, 16, buffer);
        print_string(buffer);
        print_string(" ");
    }

    for(;;);
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

char bchars[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
void int_to_string(uint32_t i, uint8_t base, char* buffer) {
    char tbuf[32];
    int pos = 0;
    int opos = 0;
    int top = 0;

    if (i == 0 || base > 16) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    while (i != 0) {
        tbuf[pos] = bchars[i % base];
        pos++;
        i /= base;
    }
    top=pos--;
    for (opos=0; opos<top; pos--,opos++) {
        buffer[opos] = tbuf[pos];
    }
    buffer[opos] = 0;
}

uint16_t read_edid(uint16_t dest_addr) {
    uint16_t output = 0;
    asm(
        "mov %0, %%di;"
        "mov $0x4f15, %%ax;"
        "mov $0x01, %%bl;"
        "xor %%cx, %%cx;"
        "xor %%dx, %%dx;"
        "int $0x10"
        : "=a" (output) // ax register
        : "r" (dest_addr)
    );
    return output;
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
