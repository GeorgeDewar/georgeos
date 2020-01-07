#include "video.h"

#define SET_VIDEO_MODE        0x00
#define WRITE_CHARACTER_TTY   0x0E

#define TEXT_80_25_16         0x03
#define GFX_80_25_16          0x10

#define COLOR_RED             0x04
#define COLOR_YELLOW          0x0E
#define COLOR_WHITE           0x0F

void clearScreen() {
   #asm
      mov ah, SET_VIDEO_MODE
      mov al, TEXT_80_25_16
      int 0x10
   #endasm
}

void printChar(char c) {
   #asm
      mov ah, WRITE_CHARACTER_TTY
      mov al, [_c]                   ; character to write
      mov bh, 0                        ; page number
      mov bl, COLOR_YELLOW             ; foreground colour
      int 0x10
   #endasm
}
