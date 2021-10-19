[bits 16]

; 1024x600

infoblock equ 0xC000
modeblock equ 0x9000

; Actually 512 bytes total
struc vbe_info_block
     .vbe_signature          resb 4
     .vbe_version            resb 2
     .oem_string_ptr_offset  resb 2
     .oem_string_ptr_segment resb 2
     .capabilities           resb 4
     .video_mode_ptr_offset  resw 1
     .video_mode_ptr_segment resw 1
     .total_memory           resb 2
endstruc

struc vbe_mode_info
    .attributes resw 1;             // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    .window_a resb 1;               // deprecated
    .window_b resb 1;               // deprecated
    .granularity resw 1;            // deprecated; used while calculating bank numbers
    .window_size resw 1;
    .segment_a resw 1;
    .segment_b resw 1;
    .win_func_ptr resd 1;           // deprecated; used to switch banks from protected mode without returning to real mode
    .pitch resw 1;                  // number of bytes per horizontal line
    .width resw 1;                  // width in pixels
    .height resw 1;                 // height in pixels
    .w_char resb 1;                 // unused...
    .y_char resb 1;                 // ...
    .planes resb 1;
    .bpp resb 1;                    // bits per pixel in this mode
    .banks resb 1;                  // deprecated; total number of banks in this mode
    .memory_model resb 1;
    .bank_size resb 1;              // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    .image_pages resb 1;
    .reserved0 resb 1;

    .red_mask resb 1;
    .red_position resb 1;
    .green_mask resb 1;
    .green_position resb 1;
    .blue_mask resb 1;
    .blue_position resb 1;
    .reserved_mask resb 1;
    .reserved_position resb 1;
    .direct_color_attributes resb 1;

    .framebuffer resd 1;            // physical address of the linear frame buffer; write here to draw to the screen
    .off_screen_mem_off resd 1;
    .off_screen_mem_size resw 1;    // size of memory in the framebuffer but not being displayed on the screen
    .reserved1 resb 206;
endstruc

    ; Write 'VBE2' into the infoblock location
    mov eax, "VBE2"
    mov [infoblock + vbe_info_block.vbe_signature], eax

    mov si, identifying_modes
    call print_string

    ; Retrieve a list of available modes
    mov ax, 0
    mov es, ax
    mov di, infoblock
    mov ax, 0x4f00
    int 0x10

    ; Display an error if VESA is not supported
    cmp ax, 0x004f
    jnz no_VESA

    ; Adapter name ptr is at 0x10006
    ; Dereference address of mode list
    mov bx, [infoblock + vbe_info_block.video_mode_ptr_offset]
    mov [vesa_mode_list], bx ; store for safekeeping
    
    ; Dereference first mode
modeloop:
    mov cx, [bx]
    push cx
    ; call print_hex_word
    ; call print_dot
    cmp cx, -1      ; the value that indicates the end of the list
    jz modeloop_end ; if we see it, stop looping

    push bx ; in case it gets overwritten by the int 0x10 call

    ; Get mode information for that mode
    mov ax, 0x4f01 ; get vesa mode information
    mov di, modeblock
    int 0x10

    pop bx
    inc bx

    ; Check if this is a packed pixel or direct color mode
    ; mov ax, [modeblock + vbe_mode_info.memory_model]
    ; cmp ax, 4
    ; jz modeloop
    ; cmp ax, 6
    ; jz modeloop

    ; Print mode number
    ; push bx
    ; call print_hex_word
    ; call print_dot

    ; Check that it's a graphics mode with linear framebuffer support
    mov ax, [modeblock + vbe_mode_info.attributes]
    ; push ax
    ; call print_hex_word
    ; call print_dot
    and ax, 0x90
    cmp ax, 0x90
    jnz modeloop

    ; Check for 16 bpp
    mov ax, 0
    mov al, byte [modeblock + vbe_mode_info.bpp]
    cmp ax, 16
    jnz modeloop

    ; Get width
    mov ax, [modeblock + vbe_mode_info.width]
    call print_int
    mov si, str_x
    call print_string

    ; Get height
    mov ax, [modeblock + vbe_mode_info.height]
    call print_int
    
    mov si, new_line
    call print_string

    ; Loop again to check the next mode
    jmp modeloop

modeloop_end:
    jmp vesa_end

no_VESA:
    mov si, no_vesa
    call print_string
    jmp $

vesa_end:
    mov ax, 0x0111
    mov [selected_mode], ax ; 640x480x16
    ; push ax
    ; call print_hex_word
    ; call pause

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
