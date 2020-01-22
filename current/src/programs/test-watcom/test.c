void appMain(void);
// void printChar(char c);
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

// void printChar(char c)
// {
//     __asm {
//         mov ah, 0x0E
//         mov al, [c]
//         mov bh, 0                        ; page number
//         mov bl, 0x0F
//         int 0x10
//     }
// }
