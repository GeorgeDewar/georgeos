org 0x7C00

mov ah, 0x0E        ; Function code
mov bh, 0           ; Page number

; Write an H
mov al, 'H'
int 10h

; Write an i
mov al, 'i'
int 10h

jmp $

times 510-($-$$) nop
dw 0xAA55
