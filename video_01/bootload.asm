org 0x7C00

mov ah, 0x0E
mov bh, 0

mov al, 'H'
int 10h

mov al, 'i'
int 10h

times 512 - 2 - ($ - $$) nop
dw 0xAA55
