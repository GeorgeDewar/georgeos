#define syscall(functionCode) __asm {       \
        __asm push bp               \
        __asm mov bp, functionCode  \
        __asm int 0x21              \
        __asm pop bp                \
    }                               
    

void print()        { syscall(0x00) }
void println()      { syscall(0x01) }
void printChar()    { syscall(0x02) }
void readString()   { syscall(0x03) }
void readChar()     { syscall(0x04) }
