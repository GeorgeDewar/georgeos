void appMain(void);
void printChar(char c);
void println(char* s);

char* str = "Hello, GeorgeOS!";

void appMain(void)
{
    printChar('G');
    println(str);
}

void println(char* s) {
    // printChar(s[0]);
    s++;
   while(*s != 0) {
      printChar(*s);
      s++;
   }
   printChar(0x0D);
   printChar(0x0A);
}

void printChar(char c)
{
    __asm {
        mov ah, 0x0E
        mov al, [c]
        mov bh, 0                        ; page number
        mov bl, 0x0F
        int 0x10
    }
}
