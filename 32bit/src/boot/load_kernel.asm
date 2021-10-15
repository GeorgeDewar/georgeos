; -----------------------------------------------------------------------------
; Routine: load_kernel
;
; Use the FAT (already in memory) to locate each cluster of the file, and load
; it into the destination location as we go.
; -----------------------------------------------------------------------------
[bits 16]

    ; Find the first cluster number of our kernel from the directory structure
    mov ax, word [DISK_BUFFER + 26]  ; Offset 26 contains 1st cluster of file; first load into AX
    mov word [cluster], ax           ; Then load into RAM

; Now we must load the kernel file from the disk. Here's how we find out where it starts:
; FAT cluster 0 = media descriptor = 0F0h
; FAT cluster 1 = filler cluster = 0FFh
; Cluster start = ((cluster number) - 2) * SectorsPerCluster + (start of user)
;               = (cluster number) + 31

load_file_sector:
    mov ax, word [cluster]      ; Convert cluster number to logical sector
    add ax, 33

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
