#include "system.h"

#define TASKBAR_HEIGHT  10

void main () {
    init_serial(115200);
    default_graphics_device = &vesa_graphics_device;
    default_graphics_device->clear_screen();

    printf("Kernel loaded successfully. Welcome to GeorgeOS!\n");
    fprintf(stddebug, "Serial connected\n");

    // Set up interrupts
    idt_install();
    irq_install();

    // Only now is it safe to enable interrupts
    __asm__ __volatile__ ("sti");

    // Initialise hardware that relies on interrupts
    timer_install();
    ps2_keyboard_install();
    
    install_floppy();
    ResetFloppy();
    // read_sector_lba(20);

    // printf("Floppy read: ");
    // printf((char *) FLOPPY_BUFFER);
    // printf("\n");

    printf("Dir:\n");
    DirEntry dirbuf[16];
    uint16_t num_entries;
    list_dir("path", dirbuf, &num_entries);
    for (int i=0; i<num_entries; i++) {
        printf(dirbuf[i].filename);
    }

    // Start our shell
    shell_main();

    for(;;);
}

/** Handle an unrecoverable error by stopping the system */
void die(char* message) {
    // Print the message - may not be seen if rendering system isn't working
    printf(message);
    printf("\nSystem halted");
    // Hang so that we can debug
    for(;;);
}
