#define SYSCALL_PRINT       0x00
#define SYSCALL_PRINTLN     0x01
#define SYSCALL_PRINTCHAR   0x02
#define SYSCALL_READLN      0x03
#define SYSCALL_READCHAR    0x04

void print() {
   __asm {
       mov bp, SYSCALL_PRINT
       int 0x21
   }
}

void println() {
   __asm {
       mov bp, SYSCALL_PRINTLN
       int 0x21
   }
}

void printChar() {
   __asm {
       mov bp, SYSCALL_PRINTCHAR
       int 0x21
   }
}

void readString() {
   __asm {
       mov bp, SYSCALL_READLN
       int 0x21
   }
}

void readChar() {
   __asm {
       mov bp, SYSCALL_READCHAR
       int 0x21
   }
}
