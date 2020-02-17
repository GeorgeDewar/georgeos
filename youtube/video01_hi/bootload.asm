org 0x7C00 ; Tell the assembler where this code will be loaded

mov ah, 0x0E                    ; Function code
mov bh, 0                       ; Page number

; Write an H to the screen
mov al, 'H'                     ; Move character code into AL
int 10h                         ; Invoke BIOS function

; Write an I to the screen
mov al, 'i'
int 10h

jmp $                           ; Endless loop

times 510-($-$$) nop            ; Add lots of nops (byte 0x90)
dw 0xAA55                       ; Add the bootloader signature
