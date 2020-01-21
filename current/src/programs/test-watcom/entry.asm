.model small
.code ENTRY
    extern _appMain : proc

    mov ah, 0Eh             ; Print a dot for debugging purposes
    mov al, '.'
    int 10h

    mov ax, cs
    add ax, @data
    ;add ax, 4000h           ; 0x2000 seg for kernel, plus 0x8000 offset for app
    mov ds, ax
    ; mov es, ax

    call _appMain            ; Jump to the main function in our C code

    mov ah, 0Eh             ; Print a dot for debugging purposes
    mov al, '/'
    int 10h

    retf
end
