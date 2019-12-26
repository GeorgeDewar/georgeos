void clearScreen();
void printchar(char c);
void print(char *s);

char *hello = "Welcome to GeorgeOS!";

void kernelMain(void) {
   clearScreen();
   print(hello);

   while(1==1) {}
}

void clearScreen() {
   __asm {
      mov ah, 0x00
      mov al, 0x03  ; text mode 80x25 16 colours
      int 0x10
   }
}

void print(char* s) {
   while(*s != 0) {
      printchar(*s);
      s++;
   }
}

void printchar(char c) {
   __asm {
      mov ah, 0Eh
      mov al, [c]
      mov bh, 0
      mov bl, 0Fh
      int 10h
   }
}
