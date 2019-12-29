#include "disk.h"

#define BYTES_PER_SECTOR        512
#define NUM_RESERVED_SECTORS    1
#define NUMBER_OF_FATS          2
#define SECTORS_PER_FAT         9
#define SECTORS_PER_TRACK       18
#define ROOT_DIR_ENTRIES        224
#define DIR_ENTRY_SIZE          32

#define ROOT_DIR_START          NUM_RESERVED_SECTORS + (NUMBER_OF_FATS * SECTORS_PER_FAT)
#define ROOT_DIR_LENGTH         (ROOT_DIR_ENTRIES * DIR_ENTRY_SIZE) / BYTES_PER_SECTOR

#define FN_02H_READ_SECTORS_FROM_DRIVE  0x02

char readRoot(char* buffer) {
    char returnCode = -1;
    char head, cylinder, sector;

    int lba = ROOT_DIR_START;

    head = (lba % (SECTORS_PER_TRACK * 2)) / SECTORS_PER_TRACK;
	cylinder = (lba / (SECTORS_PER_TRACK * 2));
	sector = (lba % SECTORS_PER_TRACK + 1);

    __asm {
        mov ah, FN_02H_READ_SECTORS_FROM_DRIVE
        mov al, 1     ; sectors to read
        mov dl, 0     ; drive number

        mov dh, [head]
        mov cl, [sector]
        mov ch, [cylinder]

        mov bx, [buffer]           ; set ES:BX to point to our buffer

        int 13h

        mov [returnCode], ah
    }

    return returnCode;
}
