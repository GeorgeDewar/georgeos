void getString(char* buffer);
char getchar();

#define FN_00_READ_KEY_PRESS 0x00

void getString(char* buffer) {
    int index = 0;
    char c;
    while((c = getchar()) != '\r') {
        buffer[index] = c;
        index++;
    }
    buffer[index] = 0; // NULL byte to end string
}

char getchar() {
    char c;
   __asm {
      mov ah, FN_00_READ_KEY_PRESS
      int 16h
      mov [c], al                      ; ASCII character code of key pressed
   }
   return c;
}
