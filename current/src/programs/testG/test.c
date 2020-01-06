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
#asm
	push bp
    mov ah, #$0E
	mov bp, sp
	mov al, [bp+4]
    int 0x10
	pop bp
#endasm
}
