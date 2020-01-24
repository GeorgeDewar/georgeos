void shutdown() {
    __asm {
        // http://www.ctyme.com/intr/rb-1401.htm
        mov ax, 0x5307          // APM 1.0+ - Set power state
        mov bx, 0x0001          // Device ID - all devices for which the system BIOS manages power
        mov cx, 0x0003          // System state - off
        int 0x15
        
        ret                     // Only applicable if interrupt doesnt work
    }
}
