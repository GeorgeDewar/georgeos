char getchar();

#define FN_00_READ_KEY_PRESS 0x00

char getchar() {
    char c;
   __asm {
      mov ah, FN_00_READ_KEY_PRESS
      int 16h
      mov [c], al                      ; ASCII character code of key pressed
   }
   return c;
}
