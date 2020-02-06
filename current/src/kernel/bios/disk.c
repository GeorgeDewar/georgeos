#include "disk.h"

#ifndef SECTORS_PER_TRACK
    #define SECTORS_PER_TRACK 0 // Redefined at build time; defined here for compiler happiness
#endif

#define FN_02H_READ_SECTORS_FROM_DRIVE  0x02

char readSectorsLBA(int lba, char numSectors, int segment, char* buffer) {
    char head, cylinder, sector;

    head = (lba % (SECTORS_PER_TRACK * 2)) / SECTORS_PER_TRACK;
    cylinder = (lba / (SECTORS_PER_TRACK * 2));
    sector = (lba % SECTORS_PER_TRACK + 1);

    return readSectors(head, cylinder, sector, numSectors, segment, buffer);
}

// Private functions

static char readSectors(char head, char cylinder, char sector, char numSectors, int segment, char *buffer) {
    char returnCode = -1;
    
    // If segment is zero, use DS, else use provided segment
    if(segment > 0) {
        __asm {
            mov ax, [segment]
            mov es, ax
        }
    } else {
        __asm {
            mov ax, ds
            mov es, ax
        }
    }

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
