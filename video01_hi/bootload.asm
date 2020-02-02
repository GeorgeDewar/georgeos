BITS 16
org 0x7C00

mov ah, 0Eh
mov bh, 0

mov al, 'H'
int 10h

mov al, 'i'
int 10h

jmp $

times 510-($-$$) nop
dw 0xAA55
