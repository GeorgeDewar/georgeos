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


digits db '0123456789ABCDEF'

; Maximum length, 32 bits formatted as binary and a null-terminator.
output db '00000000000000000000000000000000', 0


; itoa(dword number, byte width, byte radix)
; Format number as string of width in radix. Returns pointer to string.
itoa:
  push bp
  mov bp, sp
  push bx
  push si
  push di

  ; Start at end of output string and work backwards (std)
  lea di, [output + 31]
  std

  .loop:
    ; Clear remainder / upper bits of dividend.
    xor dx, dx

    ; Divide number by radix.
    div bx

    ; Use remainder to set digit in output string.
    push bx
    mov bx, dx
    lea si, [digits + bx]
    movsb
    pop bx

    loop .loop

  ; The last movsb brought us too far back.
  lea ax, [di + 1]

  cld   ; clear the direction flag so things count forwards again

  ; Actually print
  mov si, ax ; pointer to str in ax
  call print_string

  pop di
  pop si
  pop bx
  mov sp, bp
  pop bp
  ret
