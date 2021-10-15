[bits 16]

modeblock equ 0x9000

    ; TODO: Use a more sophisticated, reliable means of mode setting
    ; 0x0111 = 640x480x16bpp
    ; 0x0114 = 800x600x16bpp
    ; 0x0117 = 1027x768x16bpp
    mov ax, 0
    mov es, ax
    mov ax, 0x4f01 ; get vesa mode information
    mov cx, 0x0114 ; 1024*768*64K linear frame buffer
    mov di, modeblock
    int 0x10

    mov esi, [modeblock+0x28] ; save frame buffer base
    mov ax, 0x4f02 ; set vesa mode
    mov bx, 0x0114 ; mode, as per before
    int 0x10
