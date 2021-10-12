[bits 16]

pause:
    push ax
    mov si, press_any_key
    call print_string
    mov ax, 0
    int 16h                     ; Wait for keystroke
    pop ax
    ret

press_any_key     db "Press any key to continue...", 0
