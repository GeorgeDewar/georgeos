#include "system.h"

/*
    Memory map:
    ---------------------------------------------
    0x    1000   Floppy DMA buffer
    0x    9000   VESA mode information block
    0x   20000   Kernel
    0x   40000   Kernel stack ^
    0x   80000   Userspace program
    0x  A00000   Kernel Heap
    0x  B00000   Userspace Heap
    0x 7A12000   Top of physical memory (128MB)
    0xFD000000   Video memory (maybe)
*/

FileSystem floppy0_fs;
FileHandle open_files[16];

void main () {
    init_serial(115200);
    default_graphics_device = &vesa_graphics_device;
    default_graphics_device->clear_screen();

    open_files[stdin].type = STREAM;
    open_files[stdin].stream_device = sd_screen_console;
    open_files[stdout].type = STREAM;
    open_files[stdout].stream_device = sd_screen_console;
    open_files[stderr].type = STREAM;
    open_files[stderr].stream_device = sd_screen_console;
    open_files[stddebug].type = STREAM;
    open_files[stddebug].stream_device = sd_com1;

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
    
    // Initialise disk subsystem
    install_floppy();
    ResetFloppy();
    // Create a FileSystem instance for FAT12 on floppy0
    fs_fat12.init(&floppy0, &floppy0_fs);
    strcpy("/fd0", cwd);

    // Start shell from disk
    printf("Loading SHELL.EXE... ");
    if(!exec("/SHELL.EXE")) {
        printf("Failed to execute shell!\n");
    };

    for(;;);
}

bool exec(char* filename) {
    int (*program)(void) = (int (*)(void)) 0x80000;
    uint8_t *buffer = (uint8_t *) 0x80000;
    uint16_t length;
    // Read the program into memory - this will replace the shell!
    if (!read_file(filename, buffer, &length)) return FAILURE;
    printf("Read %d bytes\n", length);
    program();

    // Read the shell back in so when we return to it, it's there
    read_file("/SHELL.EXE", buffer, &length);
    return SUCCESS;
}

/** Handle an unrecoverable error by stopping the system */
void die(char* message) {
    // Print the message - may not be seen if rendering system isn't working
    printf(message);
    printf("\nSystem halted");
    // Hang so that we can debug
    for(;;);
}
