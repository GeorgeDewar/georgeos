; ==================================================================
; The GeorgeOS simple bootloader
;
; Based on the MikeOS bootloader. It scans the FAT12
; floppy for KERNEL.BIN (the GeorgeOS kernel), loads it and executes it.
; This must grow no larger than 512 bytes (one sector), with the final
; two bytes being the boot signature (AA55h). Note that in FAT12,
; a cluster is the same as a sector: 512 bytes. KERNEL.BIN must be the
; first file in the root directory.
; ==================================================================

    BITS 16

    %define LOAD_ADDRESS    7c00h          ; This is where the BIOS puts us
    org LOAD_ADDRESS                       ; Assume all code is at this offset

    %define FAT             9E00h          ; Where in memory to put the FAT

    %define KERNEL_SEGMENT  2000h          ; Where we are going to load the kernel
    %define KERNEL_OFFSET   0000h          ;

    jmp short bootloader_start  ; Jump past disk description section
    nop                         ; Pad out before disk description

; ------------------------------------------------------------------
; Disk description table, to make it a valid floppy
; Values are those used by IBM for 1.44 MB, 3.5" diskette

OEMLabel            db "GEORGEOS"     ; Disk label
BytesPerSector      dw 512            ; Bytes per sector
SectorsPerCluster   db 1              ; Sectors per cluster
ReservedForBoot     dw 1              ; Reserved sectors for boot record
NumberOfFats        db 2              ; Number of copies of the FAT
RootDirEntries      dw 224            ; Number of entries in root dir
                                      ; (224 * 32 = 7168 = 14 sectors to read)
LogicalSectors      dw 2880           ; Number of logical sectors
MediumByte          db 0F0h           ; Medium descriptor byte
SectorsPerFat       dw 9              ; Sectors per FAT
SectorsPerTrack     dw SECTORS_PER_TRACK ; Varies by disk size, set externally
Sides               dw 2              ; Number of sides/heads
HiddenSectors       dd 0              ; Number of hidden sectors
LargeSectors        dd 0              ; Number of LBA sectors
DriveNo             dw 0              ; Drive No: 0
Signature           db 41             ; Drive signature: 41 for floppy
VolumeID            dd 00000000h      ; Volume ID: any number
VolumeLabel         db "GEORGEOS   "  ; Volume Label: any 11 chars
FileSystem          db "FAT12   "     ; File system type: don't change!

; ------------------------------------------------------------------
; Main bootloader code
;
; Memory Map:
; 0000 - 7BFF = Stack       (grows downwards)
; 7C00 - 7DFF = Bootloader  (512 bytes)
; 7E00 - 9DFF = Disk Buffer (8KB)
; 9E00 - AFFF = FAT         (4,608 bytes)
; B000 - 

bootloader_start:
    ; Set all segments to zero so that we only need to use offsets
    mov ax, 0                   ; Load 0 into ax
    mov ds, ax                  ; Set data segment to 0
    mov es, ax                  ; Set extra segment to 0 (used for disk access)
    mov ss, ax                  ; Set stack segment to 0
    mov bp, 0x9000
    mov sp, bp                  ; Set stack pointer to LOAD_ADDRESS so stack is below bootloader

    mov ah, 0x00                ; Set video mode to clear the screen
    mov al, 0x03                ; TEXT 80/25/16
    int 10h

    mov si, bootloader_hi
    call print_string

; Load the rest of our bootloader

    mov ax, 1                   ; first sector to read
    call l2hts
    mov si, continuation ; where to load to
    mov bx, si
    mov ah, 2                   ; Params for int 13h: read floppy sectors
    mov al, 1                   ; read one more sector
    stc                         ; Set carry bit; a few BIOSes do not set properly on error
    int 13h                     ; Read sectors using BIOS

    jnc print_cool_string          ; If read went OK, skip ahead
    jmp fatal_error             ; Else it's a fatal read error

print_cool_string:
    mov si, cool_string
    call print_string
    jmp continuation

; First, we need to load the root directory from the disk. Technical details:
; Start of root = ReservedForBoot + NumberOfFats * SectorsPerFat = logical 19
; Number of root = RootDirEntries * 32 bytes/entry / 512 bytes/sector = 14
; Start of user data = (start of root) + (number of root) = logical 33

    mov ax, 19                  ; Root dir starts at logical sector 19
    call l2hts                  ; Calculate head/track/sector

    mov si, buffer              ; Set ES:BX to point to our buffer (see end of code)
    mov bx, si

    mov ah, 2                   ; Params for int 13h: read floppy sectors
    mov al, 14                  ; And read 14 of them

    stc                         ; Set carry bit; a few BIOSes do not set properly on error
    int 13h                     ; Read sectors using BIOS

    jnc check_filename          ; If read went OK, skip ahead
    jmp fatal_error             ; Else it's a fatal read error

check_filename:
    mov di, buffer
    mov si, kern_filename       ; Start searching for kernel filename
    mov cx, 11                  ; Repeat character comparison 11 times
    rep cmpsb
    je found_file_to_load       ; Success; the filename matches; jump ahead

    mov si, wrong_file          ; The filename did not match; print error and stop
    call print_string
    jmp $

found_file_to_load:
    mov si, found_kernel
    call print_string

    ; Try to load the FAT into RAM
    mov ax, 1                   ; Sector 1 = first sector of first FAT
    call l2hts                  ; Calculate head/track/sector

    mov di, FAT                 ; Set ES:BX to point to our FAT buffer
    mov bx, di

    mov ah, 2                   ; int 13h params: read (FAT) sectors
    mov al, 9                   ; All 9 sectors of 1st FAT

    stc                         ; Set carry bit; a few BIOSes do not set properly on error
    int 13h                     ; Read sectors using the BIOS

    jnc read_fat_ok             ; If read went OK, skip ahead
    call fatal_error            ; Else it's a fatal read error

read_fat_ok:
    mov si, loading_kernel
    call print_string

    ; Find the first cluster number of our kernel from the directory structure
    mov ax, word [buffer + 26]  ; Offset 26 contains 1st cluster of file; first load into AX
    mov word [cluster], ax      ; Then load into RAM


; ------------------------------------------------------------------
; BOOTLOADER SUBROUTINES

fatal_error:
    mov si, disk_error
    call print_string

pause:
    push ax
    mov ax, 0
    int 16h                     ; Wait for keystroke
    pop ax
    ret

print_dot:
    push ax
    mov ah, 0Eh                 ; int 10h teletype function
    mov al, '.'                 ; char to print
    int 10h                     ; Otherwise, print it
    pop ax
    ret

; Define some constants
VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK equ 0x0f

; prints a null - terminated string pointed to by EDX
print_string_pm:
    pusha
    mov edx , VIDEO_MEMORY ; Set edx to the start of vid mem.

print_string_pm_loop:
    mov al , [ ebx ] ; Store the char at EBX in AL
    mov ah , WHITE_ON_BLACK ; Store the attributes in AH
    cmp al , 0 ; if (al == 0) , at end of string , so
    je print_string_pm_done ; jump to done
    mov [edx] , ax ; Store char and attributes at current
    ; character cell.
    add ebx , 1 ; Increment EBX to the next char in string.
    add edx , 2 ; Move to next character cell in vid mem.
    jmp print_string_pm_loop ; loop around to print the next char.

print_string_pm_done:
    popa
    ret ; Return from the function

; -----------------------------------------------------------------------------
; Routine: print_string
;
; Print the string pointed to by SI to the screen using int 10h
; -----------------------------------------------------------------------------

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

; -----------------------------------------------------------------------------
; Routine: l2hts
;
; Calculate head, track and sector for int 13h usage from a logical sector
; number (AX). Output values are placed in the appropriate registers for int
; 13h (CL, CH, DL, DH).
; -----------------------------------------------------------------------------
l2hts:
    push bx
    push ax

    mov bx, ax                  ; Save logical sector

    mov dx, 0                   ; First the sector
    div word [SectorsPerTrack]
    add dl, 01h                 ; Physical sectors start at 1
    mov cl, dl                  ; Sectors belong in CL for int 13h
    mov ax, bx

    mov dx, 0                   ; Now calculate the head
    div word [SectorsPerTrack]
    mov dx, 0
    div word [Sides]
    mov dh, dl                  ; Head/side
    mov ch, al                  ; Track

    pop ax
    pop bx

    mov dl, byte [bootdev]      ; Set correct device

    ret

; -----------------------------------------------------------------------------
; Strings and variables
; -----------------------------------------------------------------------------

    kern_filename  db "KERNEL  BIN"    ; Kernel filename

    bootloader_hi  db "Starting GeorgeOS", 0
    found_kernel   db "Loading FAT", 0
    loading_kernel db "Loading Kernel",  0
    MSG_PROT_MODE db " Successfully landed in 32 - bit Protected Mode " , 0
    jumping        db "Jumping to Kernel", 0
    new_line       db 0

    disk_error     db "Disk read error", 0
    wrong_file     db "File 0 is not KERNEL.BIN", 0
    
    bootdev        db 0     ; Boot device number
    cluster        dw 0     ; Cluster of the file we want to load
    pointer        dw 0     ; Pointer into Buffer, for loading kernel


; ------------------------------------------------------------------
; END OF BOOT SECTOR PART 1 AND BUFFER START

    times 510-($-$$) db 0    ; Pad remainder of boot sector with zeros
    dw 0AA55h        ; Boot signature (DO NOT CHANGE!)

continuation:
    mov si, bootloader_hi
    call print_string
    jmp switch_prot

    cool_string     db "Cool string", 0

; Now we must load the kernel file from the disk. Here's how we find out where it starts:
; FAT cluster 0 = media descriptor = 0F0h
; FAT cluster 1 = filler cluster = 0FFh
; Cluster start = ((cluster number) - 2) * SectorsPerCluster + (start of user)
;               = (cluster number) + 31

load_file_sector:
    mov ax, word [cluster]      ; Convert cluster number to logical sector
    add ax, 31

    call l2hts                  ; Make appropriate params for int 13h

    mov ax, KERNEL_SEGMENT      ; Set buffer to where we want the kernel to load, offset by
    mov es, ax                  ; the current pointer (location of the next cluster)
    mov bx, word [pointer]

    mov ah, 2                   ; int 13h floppy read params
    mov al, 1                   ; Load one sector

    stc
    int 13h

    jnc calculate_next_cluster  ; If there's no error...
    jmp fatal_error             ; Else, fatal read error

; In the FAT, cluster values are stored in 12 bits, so we have to do a bit of maths to work out
; whether we're dealing with a byte and 4 bits of the next byte -- or the last 4 bits of one byte
; and then the subsequent byte!

calculate_next_cluster:
    mov ax, [cluster]

    call print_dot              ; One dot for each cluster loaded, so we can see what's happening

    ; Set AX to [cluster] * 3 / 2, i.e. the memory address of the word containing the next cluster address
    ; Set DX = [cluster] mod 2, to determine whether it is even or odd

    mov dx, 0                   ; Segment address
    mov bx, 3                   ; BX = 3
    mul bx                      ; AX = AX * BX = AX * 3
    mov bx, 2                   ; BX = 2
    div bx                      ; AX = AX / BX = AX / 2; DX = remainder

    mov si, FAT                 ; AX = word in FAT for the 12 bit entry
    add si, ax
    mov ax, word [ds:si]

    or dx, dx                   ; If DX = 0 [cluster] is even; if DX = 1 then it's odd
    jz even                     ; If [cluster] is even, drop last 4 bits of word
                                ; with next cluster; if odd, drop first 4 bits

odd:
    shr ax, 4                   ; Shift out first 4 bits (they belong to another entry)
    jmp next_cluster_cont

even:
    and ax, 0FFFh               ; Mask out final 4 bits

next_cluster_cont:
    mov word [cluster], ax      ; Store cluster number

    cmp ax, 0FF8h               ; FF8h = end of file marker in FAT12
    jae end                     ; Jump if we are at the end of the file

    add word [pointer], 512     ; Else, increase buffer pointer 1 sector length
    jmp load_file_sector        ; Load the next sector

end:
    mov si, new_line            ; Print a new line after all those dots
    call print_string

    mov si, jumping             ; Print our last status message
    call print_string

    ;call pause                  ; Wait for a keystroke

; GDT
gdt_start:

gdt_null: ; the mandatory null descriptor
    dd 0x0 ; ’dd ’ means define double word ( i.e. 4 bytes )
    dd 0x0

gdt_code: ; the code segment descriptor
    ; base =0x0 , limit =0 xfffff ,
    ; 1st flags : ( present )1 ( privilege )00 ( descriptor type )1 -> 1001 b
    ; type flags : ( code )1 ( conforming )0 ( readable )1 ( accessed )0 -> 1010 b
    ; 2nd flags : ( granularity )1 (32 - bit default )1 (64 - bit seg )0 ( AVL )0 -> 1100 b
    dw 0xffff ; Limit ( bits 0 -15)
    dw 0x0 ; Base ( bits 0 -15)
    db 0x0 ; Base ( bits 16 -23)
    db 10011010b ; 1st flags , type flags
    db 11001111b ; 2nd flags , Limit ( bits 16 -19)
    db 0x0 ; Base ( bits 24 -31)

gdt_data: ; the data segment descriptor
    ; Same as code segment except for the type flags :
    ; type flags : ( code )0 ( expand down )0 ( writable )1 ( accessed )0 -> 0010 b
    dw 0xffff ; Limit ( bits 0 -15)
    dw 0x0 ; Base ( bits 0 -15)
    db 0x0 ; Base ( bits 16 -23)
    db 10010010b ; 1st flags , type flags
    db 11001111b ; 2nd flags , Limit ( bits 16 -19)
    db 0x0 ; Base ( bits 24 -31)

gdt_end: ; The reason for putting a label at the end of the
    ; GDT is so we can have the assembler calculate
    ; the size of the GDT for the GDT decriptor ( below )
    ; GDT descriptior

gdt_descriptor:
    dw gdt_end - gdt_start - 1 ; Size of our GDT , always less one
    ; of the true size
    dd gdt_start ; Start address of our GDT

; Define some handy constants for the GDT segment descriptor offsets , which
; are what segment registers must contain when in protected mode. For example ,
; when we set DS = 0 x10 in PM , the CPU knows that we mean it to use the
; segment described at offset 0 x10 ( i.e. 16 bytes ) in our GDT , which in our
; case is the DATA segment (0 x0 -> NULL ; 0x08 -> CODE ; 0 x10 -> DATA )
CODE_SEG equ gdt_code - gdt_start
DATA_SEG equ gdt_data - gdt_start

; Switch to protected mode
switch_prot:
    cli
    lgdt [ gdt_descriptor ]
    mov eax , cr0 ; To make the switch to protected mode , we set
    or eax , 0x1 ; the first bit of CR0 , a control register
    mov cr0 , eax ; Update the control register

    jmp CODE_SEG : start_protected_mode

[bits 32]
start_protected_mode :
    mov ax , DATA_SEG ; Now in PM , our old segments are meaningless ,
    mov ds , ax ; so we point our segment registers to the
    mov ss , ax ; data selector we defined in our GDT
    mov es , ax
    mov fs , ax
    mov gs , ax
    mov ebp , 0x90000 ; Update our stack position so it is right
    mov esp , ebp ; at the top of the free space.

    mov ebx , MSG_PROT_MODE
    call print_string_pm

    jmp $

    ; Jump to entry point of loaded kernel!
    jmp KERNEL_SEGMENT:KERNEL_OFFSET

    times 1024-($-$$) db 0

buffer:                ; Disk buffer begins (8k after this, stack starts)


; ==================================================================

