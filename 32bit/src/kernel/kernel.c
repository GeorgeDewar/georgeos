#include "system.h"

void main () {
    init_serial(115200);
    vesa_clear_screen();
    vesa_print_string("Kernel loaded successfully. Welcome to GeorgeOS!\n");

    for(int i=0; i<COLS; i++) {
        print_char_fixed(' ', ROWS-1, i, 0x9f);
    }

    idt_install();
    irq_install();

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    timer_install();
    ps2_keyboard_install();

    vesa_print_string("Configuring floppy\n");
    install_floppy();
    vesa_print_string("Resetting floppy controller\n");
    ResetFloppy();
    vesa_print_string("Reading sector from floppy\n");
    read_sector_lba(20);

    vesa_print_string((char *) FLOPPY_BUFFER);
    vesa_print_string("\n");

    vesa_putchar(61,'H');
    vesa_putchar(62,'e');
    vesa_putchar(63,'l');
    vesa_putchar(64,'l');
    vesa_putchar(65,'o');


    vesa_print_string("hey man\nI'm a string...");

    // Start our shell
    shell_main();

    for(;;);
}
