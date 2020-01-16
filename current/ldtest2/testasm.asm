[bits 16]
extern _foo
global _main

_main:
    mov ax, 0x0e41
    int 0x10
    call _foo
