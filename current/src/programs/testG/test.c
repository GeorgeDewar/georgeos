void main(void);
void printChar(char c);
void println(char* s);

void main(void)
{
    println("Hello, GeorgeOS!");
}

void println(char* s) {
   while(*s != 0) {
      printChar(*s);
      s++;
   }
   printChar(0x0D);
   printChar(0x0A);
}

void printChar(char c)
{
    // asm("push bp"); // SmallerC does this for us
    asm("mov ah, 0x0E");
    asm("mov bp, sp");
    asm("mov al, [bp+4]");
    asm("int 0x10");
    // asm("pop bp"); // SmallerC does this for us with the leave instruction
}
