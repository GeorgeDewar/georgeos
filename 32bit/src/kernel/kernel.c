#include "../include/drivers/vga.h"

void _start () {
    clear_screen();
    print_string("Kernel loaded successfully. Welcome to GeorgeOS!\n");

    for(int i=0; i<COLS; i++) {
        print_char_fixed(' ', ROWS-1, i, 0x9f);
    }

    while(1) {

    }
}
