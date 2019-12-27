#include "video.h"

#define SET_VIDEO_MODE        0x00
#define WRITE_CHARACTER_TTY   0x0E

#define TEXT_80_25_16         0x03
#define GFX_80_25_16          0x10

#define COLOR_RED             0x04
#define COLOR_YELLOW          0x0E
#define COLOR_WHITE           0x0F

void clearScreen() {
   __asm {
      mov ah, SET_VIDEO_MODE
      mov al, TEXT_80_25_16
      int 10h
   }
}

void printChar(char c) {
   __asm {
      mov ah, WRITE_CHARACTER_TTY
      mov al, [c]                      ; character to write
      mov bh, 0                        ; page number
      mov bl, COLOR_YELLOW             ; foreground colour
      int 10h
   }
}
