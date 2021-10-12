; -----------------------------------------------------------------------------
; Routine: print_string
;
; Print the string pointed to by SI to the screen using int 10h
; -----------------------------------------------------------------------------
[bits 16]

print_string:
    push ax
    mov ah, 0Eh                 ; int 10h teletype function

.repeat:
    lodsb                       ; Get char from string
    cmp al, 0                   ; If char is zero, end of string
    je .done                    ; So jump to end
    int 10h                     ; Otherwise, print it
    jmp .repeat                 ; Repeat for the next character

.done:
    mov al, 0x0D                ; Print CR and NL
    int 10h
    mov al, 0x0A
    int 10h

    pop ax
    ret

; Print 16 bit value passed on stack as first parameter
; in hexadecimal. This routine will work on 80186+ processors
; Use page number and foreground color passed in second parameter

print_hex_word:
    pusha           ; Save all registers, 16 bytes total
    mov bp, sp      ; BP=SP, on 8086 can't use sp in memory operand
    mov cx, 0x0404  ; CH = number of nibbles to process = 4 (4*4=16 bits)
                    ; CL = Number of bits to rotate each iteration = 4 (a nibble)
    mov dx, [bp+18] ; DX = word parameter on stack at [bp+18] to print
    mov bx, [bp+20] ; BX = page / foreground attr is at [bp+20]

.loop:
    rol dx, cl      ; Roll 4 bits left. Lower nibble is value to print
    mov ax, 0x0e0f  ; AH=0E (BIOS tty print),AL=mask to get lower nibble
    and al, dl      ; AL=copy of lower nibble
    add al, 0x90    ; Work as if we are packed BCD
    daa             ; Decimal adjust after add.
                    ;    If nibble in AL was between 0 and 9, then CF=0 and
                    ;    AL=0x90 to 0x99
                    ;    If nibble in AL was between A and F, then CF=1 and
                    ;    AL=0x00 to 0x05
    adc al, 0x40    ; AL=0xD0 to 0xD9
                    ; or AL=0x41 to 0x46
    daa             ; AL=0x30 to 0x39 (ASCII '0' to '9')
                    ; or AL=0x41 to 0x46 (ASCII 'A' to 'F')
    int 0x10        ; Print ASCII character in AL
    dec ch
    jnz .loop       ; Go back if more nibbles to process
    popa            ; Restore all the registers
    ret
