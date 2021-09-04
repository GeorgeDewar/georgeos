; ==================================================================
; The GeorgeOS simple bootloader
;
; Based on the MikeOS bootloader. It scans the FAT12
; floppy for KERNEL.BIN (the GeorgeOS kernel), loads it and executes it.
; This must grow no larger than 512 bytes (one sector), with the final
; two bytes being the boot signature (AA55h). Note that in FAT12,
; a cluster is the same as a sector: 512 bytes. KERNEL.BIN must be the
; first file in the root directory.
;
; Memory Map:
; 0000 - 7BFF = Stack       (grows downwards)
; 7C00 - 83FF = Bootloader  (up to 2KB reserved)
; 8400 - A3FF = Disk Buffer (8KB)
; A400 - B5FF = FAT         (4,608 bytes)
; B600 - 
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

    %define DISK_BUFFER     8400h          ; Where in memory to put temporarily read sectors from disk
    %define FAT             A400h          ; Where in memory to put the FAT

    jmp short bootloader_start  ; Jump past disk description section
    nop                         ; Pad out before disk description

    %include "src/boot/disk_description_table.asm"
    %include "src/boot/gdt.asm"

; ------------------------------------------------------------------
; Main bootloader code
; ------------------------------------------------------------------

bootloader_start:
    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0                   ; Load 0 into ax
    mov ds, ax                  ; Set data segment to 0
    mov es, ax                  ; Set extra segment to 0 (used for disk access)
    mov ss, ax                  ; Set stack segment to 0
    mov sp, LOAD_ADDRESS - 1    ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader

    ; Clear the screen
    mov ah, 0x00                ; Set video mode to clear the screen
    mov al, 0x03                ; TEXT 80/25/16
    int 10h

    ; Print a message
    mov si, bootloader_hi
    call print_string

    ; Load the rest of the bootloader + FAT + root directory into memory
    ; Sector 0 is this code. We read from sector 1, which is the rest of the bootloader,
    ; then 2x FAT @ 9 sectors each (total 18), then 14 sectors for the root directory,
    ; all at once.
    mov ax, 1                   ; Root dir starts at logical sector 19
    call l2hts                  ; Calculate head/track/sector

    ; mov si, DISK_BUFFER         ; Set ES:BX to point to our buffer (see end of code)
    ; mov bx, si

    ; mov ah, 2                   ; Params for int 13h: read floppy sectors
    ; mov al, 33                  ; And read 33 of them

    ; stc                         ; Set carry bit; a few BIOSes do not set properly on error
    ; int 13h                     ; Read sectors using BIOS

    ; jnc initial_read_ok         ; If read went OK, skip ahead
    ; jmp fatal_error             ; Else it's a fatal read error

initial_read_ok:
    mov si, loading_kernel
    call print_string

    ; Switch to protected mode - after the include we are in [bits 32]
    call print_dot
    ;jmp switch_prot
    nop
    ; nop one nop here breaks it
    nop
    %include "src/boot/protected_mode.asm"

    mov ebx, MSG_PROT_MODE
    call print_string_pm

    jmp $

; [bits 16]
; ------------------------------------------------------------------
; BOOTLOADER SUBROUTINES

fatal_error:
    mov si, disk_error
    call print_string

print_dot:
    push ax
    mov ah, 0Eh                 ; int 10h teletype function
    mov al, '.'                 ; char to print
    int 10h                     ; Otherwise, print it
    pop ax
    ret

%include "src/boot/print_string.asm"

%include "src/boot/print_string_pm.asm"

%include "src/boot/l2hts.asm"


; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

    kern_filename       db "KERNEL  BIN"    ; Kernel filename

    bootloader_hi       db "Starting GeorgeOS", 0
    found_kernel        db "Loading FAT", 0
    loading_kernel      db "Loading Kernel",  0

    jumping             db "Jumping to Kernel", 0
    new_line            db 0

    disk_error          db "Disk read error", 0
    ; wrong_file          db "File 0 is not KERNEL.BIN", 0

        ; nop -- here breaks it, but one line down is fine
    ; nop
    bootdev             db 0x90     ; Boot device number
    

    cluster             dw 0     ; Cluster of the file we want to load

    pointer             dw 0     ; Pointer into Buffer, for loading kernel

; ------------------------------------------------------------------
; END OF BOOT SECTOR PART 1 AND BUFFER START

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros
    dw 0AA55h        ; Boot signature (DO NOT CHANGE!)

buffer:

    ; times 512 db 55
; ==================================================================
