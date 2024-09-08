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
; 9000 (overlapping)        VESA mode information
; A400 - B5FF = FAT         (4,608 bytes)
; B600 - 
; C000 - C200 = VESA controller info
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

    %define DISK_BUFFER     8400h          ; Where in memory to put temporarily read sectors from disk
    %define FAT             0a400h         ; Where in memory to put the FAT

    %define KERNEL_SEGMENT  2000h          ; Where we are going to load the kernel
    %define KERNEL_OFFSET   0000h          ; 2000:0000h = 20000h

    jmp short bootloader_start  ; Jump past disk description section
    nop                         ; Pad out before disk description

    
%ifdef FLOPPY
    %include "src/boot/hdd/disk_description_table.asm"
%elifdef HDD
    %include "src/boot/hdd/disk_description_table.asm"
%endif

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

    ; Show the boot device
    mov si, boot
    call print_string
    mov ax, [bootdev]
    push ax                           ; Push on stack as 1st parameter
    call print_hex_byte               ; Print 8-bit value as hex
    mov si, new_line
    call print_string

%ifdef PAUSE
    call pause
%endif

%ifdef HDD
    call check_lba
%endif

    ; Load the rest of the bootloader from sector 1
    mov bx, 1           ; start at sector 1
    mov al, 2           ; read two sectors
    mov si, stage2      ; put it at stage2
    mov cx, 0
    call read_sectors   ; call

    ; Jump to stage 2, which we just loaded
    mov si, jumping_to_pt2
    call print_string
    jmp stage2

; [bits 16]
; ------------------------------------------------------------------
; BOOTLOADER SUBROUTINES

fatal_error:
    mov si, disk_error
    call print_string
    jmp $

print_dot:
    push ax
    mov ah, 0Eh                 ; int 10h teletype function
    mov al, '.'                 ; char to print
    int 10h                     ; Otherwise, print it
    pop ax
    ret

%include "src/boot/print_string.asm"
%ifdef FLOPPY
    %include "src/boot/floppy_read.asm"
%elifdef HDD
    %include "src/boot/hdd_read.asm"
%endif
%include "src/boot/pause.asm"

; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

    bootloader_hi       db "Starting GeorgeOS", 0x0D, 0x0A, 0
    boot                db "Boot ", 0
    jumping_to_pt2      db "Entering stage 2", 0x0D, 0x0A, 0
    new_line            db 0x0D, 0x0A, 0

    disk_error          db "Disk read error", 0x0D, 0x0A, 0

    bootdev             db 0     ; Boot device number

; ------------------------------------------------------------------
; END OF BOOT SECTOR STAGE 1

    times 436-($-$$) db 0    ; Pad remainder of boot sector with zeros

partition_table: times 74 db 0
    dw 0AA55h                ; Boot signature (DO NOT CHANGE!)

; START OF STAGE 2 (loaded immediately after stage 1)
; ------------------------------------------------------------------

stage2:
    mov si, loading_root_dir
    call print_string
    
    ; Load the root directory
    ; First, we need to load the root directory from the disk. Technical details:
    ; Start of root = ReservedForBoot + NumberOfFats * SectorsPerFat = logical 19
    ; Number of root = RootDirEntries * 32 bytes/entry / 512 bytes/sector = 14
    ; Start of user data = (start of root) + (number of root) = logical 33
    mov bx, ROOT_DIR_LOCATION    ; start at sector
    mov al, SECTORS_PER_DIR      ; sectors to read
    mov si, DISK_BUFFER          ; put it in the general disk buffer
    mov cx, 0
    call read_sectors            ; call the function

    mov si, looking_for_kernel
    call print_string
    %include "src/boot/find_kernel.asm"

    %ifdef FLOPPY
        ; Load the File Allocation Table (FAT)
        mov si, loading_fat
        call print_string
        mov bx, NUM_RESERVED_SECTORS ; start at sector
        mov al, SECTORS_PER_FAT      ; sectors to read
        mov si, FAT                  ; put it in the FAT location
        call read_sectors            ; call the function
    %endif

    mov si, new_line
    call print_string

    mov si, loading_kernel
    call print_string

    %ifdef FLOPPY
        %include "src/boot/floppy/load_kernel.asm"
    %elifdef HDD
        %include "src/boot/hdd/load_kernel.asm"
    %endif

    %ifdef PAUSE
        call pause
    %endif
    %include "src/boot/vesa.asm"

switch_to_prot:
    ;mov si, switching_to_prot
    ;call print_string

    ; Switch to protected mode - after the include we are in [bits 32]
    %include "src/boot/protected_mode.asm"

    ;mov ebx, MSG_PROT_MODE
    ;call print_string_pm

    ; jmp KERNEL_OFFSET
    call 0x20000
    jmp $
    db 0xBA, 0xBA, 0xBA

    %include "src/boot/print_string_pm.asm"


; -----------------------------------------------------------------------------
; Strings and variables (stage 2)
; -----------------------------------------------------------------------------

    %include "src/boot/gdt.asm"

    kern_filename        db "KERNEL  BIN"    ; Kernel filename
    loading_root_dir     db "Loading root directory", 0x0D, 0x0A, 0
    looking_for_kernel   db "Locating KERNEL.BIN", 0x0D, 0x0A, 0
    file_not_found       db 0x0D, 0x0A, "Could not find KERNEL.BIN in root directory", 0x0D, 0x0A, 0
    loading_fat          db 0x0D, 0x0A, "Loading FAT", 0x0D, 0x0A, 0
    loading_kernel       db "Loading kernel", 0x0D, 0x0A, 0
    switching_to_prot    db "Switching to protected mode", 0x0D, 0x0A, 0
    identifying_modes    db "Identifying supported video modes", 0x0D, 0x0A, 0
    no_vesa              db "VESA is not supported", 0x0D, 0x0A, 0
    str_x                db "x", 0
    cluster              dw 0     ; Cluster of the file we want to load
    pointer              dw 0     ; Pointer into Buffer, for loading kernel

    vesa_mode_list       db 0     ; Address of mode list
    selected_mode        db 0
    ; vbe_info_block:
    ;  .vbe_signature          db "VBE2"
    ;  .vbe_version            resb 2
    ;  .oem_string_ptr_offset  resb 2
    ;  .oem_string_ptr_segment resb 2
    ;  .capabilities           resb 4
    ;  .video_mode_ptr_offset  resw 1
    ;  .video_mode_ptr_segment resw 1
    ;  .total_memory           resb 2

    times 1024-($-stage2) db 0
; ==================================================================
