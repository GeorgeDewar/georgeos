#include "keyboard.h"

char c;

char getChar() {
   #asm
      mov ah, FN_00_READ_KEY_PRESS
      int 0x16
      mov [_c], al                      ; ASCII character code of key pressed
   #endasm
   return c;
}
