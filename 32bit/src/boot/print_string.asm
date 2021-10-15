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

; Input:
; EAX = integer value to convert
; ESI = pointer to buffer to store the string in (must have room for at least 10 bytes)
; Output:
; EAX = pointer to the first character of the generated string
str_buf db '0000000000', 0
print_int:
  pusha
  mov si, str_buf
  add si,9
  mov byte [si], 0

  mov bx,10         
.next_digit:
  xor dx, dx         ; Clear edx prior to dividing edx:eax by ebx
  div bx             ; eax /= 10
  add dl,'0'          ; Convert the remainder to ASCII 
  dec esi             ; store characters in reverse order
  mov [si], dl
  test ax,ax            
  jnz .next_digit     ; Repeat until eax==0
  call print_string
  popa
  ret


; digits db '0123456789ABCDEF'

; ; Maximum length, 32 bits formatted as binary and a null-terminator.
; output db '00000000000000000000000000000000', 0


; ; itoa(dword number, byte width, byte radix)
; ; Format number as string of width in radix. Returns pointer to string.
; itoa:
;   push bp
;   mov bp, sp
;   push bx
;   push si
;   push di

;   ; Start at end of output string and work backwards.
;   lea di, [output + 32]
;   std

;   ; Load number and radix for division and iteration.
;   mov ax, [bp + 8] ; number
;   movzx bx, byte [bp + 13] ; radix

;   ; Loop width times.
;   movzx cx, byte [bp + 12] ; width

;   .loop:
;     ; Clear remainder / upper bits of dividend.
;     xor dx, dx

;     ; Divide number by radix.
;     div bx

;     ; Use remainder to set digit in output string.
;     push bx
;     mov bx, dx
;     lea si, [digits + bx]
;     movsb
;     pop bx

;     loop .loop

;   ; The last movsb brought us too far back.
;   lea ax, [di + 1]

;   cld
;   pop di
;   pop si
;   pop bx
;   mov sp, bp
;   pop bp
;   ret
