#include "keyboard.h"

char getChar() {
   char c = 0;
   __asm {
      mov ah, FN_00_READ_KEY_PRESS
      int 16h
      mov [c], al                      ; ASCII character code of key pressed
   }
   return c;
}
