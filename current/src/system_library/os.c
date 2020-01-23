#define SYSCALL_PRINT       0x01
#define SYSCALL_PRINTLN     0x00

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
