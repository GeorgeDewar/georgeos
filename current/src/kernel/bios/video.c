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

      // MOV AH, 06h    ; Scroll up function
      // XOR AL, AL     ; Clear entire screen
      // XOR CX, CX     ; Upper left corner CH=row, CL=column
      // MOV DX, 184FH  ; lower right corner DH=row, DL=column 
      // MOV BH, 1Eh    ; YellowOnBlue
      // INT 10H

      // push ds
      // push ax
      // mov ax, 0xB800
      // mov ds, ax
      
      // mov al, 'Z'
      // mov bx, 0
      // mov byte [bx], al
      // mov 0x800, al

      // pop ax
      // pop ds
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
