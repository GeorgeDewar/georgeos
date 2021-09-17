#include "system.h"

void main () {
    init_serial(115200);
    clear_screen_gfx();
    print_string("Kernel loaded successfully. Welcome to GeorgeOS!\n");

    for(int i=0; i<COLS; i++) {
        print_char_fixed(' ', ROWS-1, i, 0x9f);
    }

    idt_install();
    irq_install();

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    timer_install();
    ps2_keyboard_install();

    print_string("Configuring floppy\n");
    install_floppy();
    print_string("Resetting floppy controller\n");
    ResetFloppy();
    print_string("Reading sector from floppy\n");
    read_sector_lba(20);

    print_string((char *) FLOPPY_BUFFER);
    print_string("\n");

    extern void putchar(int row, int col, char char_num);
    putchar(1,1,'H');
    putchar(1,2,'e');
    putchar(1,3,'l');
    putchar(1,4,'l');
    putchar(1,5,'o');

    // Start our shell
    shell_main();

    for(;;);
}
