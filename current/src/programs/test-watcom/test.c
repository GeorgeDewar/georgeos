void appMain(void);
void println(char* s);

char* str = "Hello, GeorgeOS!";

void appMain(void)
{
    println(str);
}

void println(char* s) {
   __asm {
       mov bp, 0x00
       int 0x21
   }
}
