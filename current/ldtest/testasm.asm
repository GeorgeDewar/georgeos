[bits 16]
global _foo

_foo:
    mov ax, 0x0e41
    int 0x10
    jmp $
