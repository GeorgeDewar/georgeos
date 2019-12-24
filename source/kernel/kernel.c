void printchar(char c);
void print(char *s);

char *hello = "Welcome to GeorgeOS!";

void kernelMain(void) {
   printchar('.');
   printchar(0xa);
   printchar(0xd);
   
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
