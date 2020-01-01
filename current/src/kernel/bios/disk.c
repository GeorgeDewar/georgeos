#include "disk.h"

#define SECTORS_PER_TRACK       18

#define FN_02H_READ_SECTORS_FROM_DRIVE  0x02

char readSectorLBA(int lba, char* buffer) {
    char head, cylinder, sector;

    head = (lba % (SECTORS_PER_TRACK * 2)) / SECTORS_PER_TRACK;
	cylinder = (lba / (SECTORS_PER_TRACK * 2));
	sector = (lba % SECTORS_PER_TRACK + 1);

    return readSector(head, cylinder, sector, buffer);
}

char readSector(char head, char cylinder, char sector, char *buffer) {
    char returnCode = -1;
    
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
