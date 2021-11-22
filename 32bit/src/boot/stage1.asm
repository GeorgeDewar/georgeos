; ==================================================================
; The GeorgeOS bootloader
;
; Just loads the Stage2 from sector 2-5 of the boot volume, allowing
; for up to 2KB stage2 without interfering with the usual locations
; of the FSInfo and backup boot sectors under FAT32.
;
; Memory Map:
; 00000000 - 00007BFF = Stack       (grows downwards)
; 00007C00 - 00007DFF = Stage1      (512 bytes)
; 00007E00 -          = Stage2
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

    jmp short bootloader_start  ; Jump past disk description section
    nop                         ; Pad out before disk description

    times 79 db 0               ; Leave space for a FAT32 EBPB

; ------------------------------------------------------------------
; Main bootloader code
; ------------------------------------------------------------------

bootloader_start:
    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0                   ; Load 0 into ax
    mov ds, ax                  ; Set data segment to 0
    mov es, ax                  ; Set extra segment to 0 (used for disk access)
    mov ss, ax                  ; Set stack segment to 0
    mov sp, LOAD_ADDRESS        ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader

    mov [bootdev], dl           ; Save the boot device

    ; Clear the screen
    mov ah, 0x00                ; Set video mode to clear the screen
    mov al, 0x03                ; TEXT 80/25/16
    int 10h

    ; Print a message
    mov si, bootloader_hi
    call print_string

    ; Print the boot device
    mov ax, [bootdev]
    mov bx, 16
    mov cx, 2
    call itoa

    mov si, new_line
    call print_string

    ; Load the rest of the bootloader from sector 1
    mov dl, [bootdev]
    mov si, DAP
    mov ah, 42h
    stc
    int 13h
    jc fatal_error

    ; Jump to stage 2, which we just loaded
    mov si, jumping_to_pt2
    call print_string
    jmp $

; [bits 16]
; ------------------------------------------------------------------
; BOOTLOADER SUBROUTINES

fatal_error:
    mov si, disk_error
    call print_string
    xor al, al ; We only want to print the error code in AH
    shr ax, 8
    mov bx, 16
    mov cx, 2
    call itoa
    jmp $

%include "src/boot/print_string.asm"
%include "src/boot/pause.asm"

; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

    bootloader_hi       db "Starting GeorgeOS", 0x0D, 0x0A, "Boot Drive: 0x", 0
    jumping_to_pt2      db "Jumping to bootloader stage 2", 0x0D, 0x0A, 0
    new_line            db 0x0D, 0x0A, 0

    disk_error          db "Disk read error", 0x0D, 0x0A, 0

    bootdev             db 0     ; Boot device number

; Disk address packet
DAP:
                        db 10h, 0
    sectors_to_read     dw 1 ; Number of sectors to be loaded
    read_dest           dd bootend
    read_from_sector    dq 2

; ------------------------------------------------------------------
; END OF BOOT SECTOR STAGE 1

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros
    dw 0AA55h                ; Boot signature (DO NOT CHANGE!)

bootend:
    ; Print MDP to upper left of screen in white on light magenta
    ;mov ax, 0xb800
    ;mov es, ax
    ;mov word [es:0x0000], 0x57<<8 | 'M'
    ;mov word [es:0x0002], 0x57<<8 | 'D'
    ;mov word [es:0x0004], 0x57<<8 | 'P'

    ; Infinite loop so we don't have the CPU wander memory
    ;cli
endloop:
    ;hlt
    ;jmp endloop
