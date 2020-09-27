#include "disk.h"
#include "../components/console.h"
#include "video.h"

#ifndef SECTORS_PER_TRACK
    #define SECTORS_PER_TRACK 0 // Redefined at build time; defined here for compiler happiness
#endif

#define FN_02H_READ_SECTORS_FROM_DRIVE  0x02

char resetDisk() {
    __asm {
        mov ah, 0
        mov dl, 0
        int 13h
    }
}

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
    
    printInt(head); //1
    printChar(',');
    printInt(cylinder); // 0
    printChar(',');
    printInt(sector); // 2
    printChar(',');
    printInt(numSectors); // 1
    printChar(',');
    printInt(segment);
    printNl();

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
        jc error

        success:
        mov [returnCode], 0
        jmp done

        error:

        mov [returnCode], ah

        done:
    }

    return returnCode;
}
