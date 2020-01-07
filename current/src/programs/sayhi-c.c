void main(void);
void printChar(char c);
void println(char* s);
void _cstart(void);

void _cstart(void) {

}

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
    __asm{
        mov ah, 0x0E
        mov al, [c]
        int 0x10
    }
}
