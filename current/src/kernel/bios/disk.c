#include "disk.h"

#define SECTORS_PER_TRACK       18

#define FN_02H_READ_SECTORS_FROM_DRIVE  0x02

char readSectorsLBA(int lba, char numSectors, char* buffer) {
    char head, cylinder, sector;

    head = (lba % (SECTORS_PER_TRACK * 2)) / SECTORS_PER_TRACK;
	cylinder = (lba / (SECTORS_PER_TRACK * 2));
	sector = (lba % SECTORS_PER_TRACK + 1);

    return readSectors(head, cylinder, sector, numSectors, buffer);
}

// Private functions

static char readSectors(char head, char cylinder, char sector, char numSectors, char *buffer) {
    char returnCode = -1;
    
    __asm {
        mov ah, FN_02H_READ_SECTORS_FROM_DRIVE
        mov al, [numSectors]       ; sectors to read
        mov dl, 0                  ; drive number

        mov dh, [head]
        mov cl, [sector]
        mov ch, [cylinder]

        mov bx, [buffer]           ; set ES:BX to point to our buffer

        int 13h

        mov [returnCode], ah
    }

    return returnCode;
}
