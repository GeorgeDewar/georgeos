void printchar(char c);
void print(char *s);

char *hello = "Welcome to GeorgeOS";

void kernelMain(void) {
   __asm {
      mov ah, 0Eh
      mov al, 'G'
      mov bh, 0
      mov bl, 0Fh
      int 10h
   }
   __asm {
      mov ah, 0Eh
      mov al, '1'
      mov bh, 0
      mov bl, 0Fh
      int 10h
   }
   printchar('C');
   print(hello);
   while(1==1) {

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
