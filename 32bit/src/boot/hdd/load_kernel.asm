[bits 16]

    ; Find the first cluster number of our kernel from the directory structure
    mov ax, word [DISK_BUFFER + 26]  ; Offset 26 of the directory entry contains 1st cluster of file; first load into AX
    mov word [cluster], ax           ; Then load into RAM

    mov ax, word [DISK_BUFFER + 28]  ; Offset 28 for the file size (32-bit integer)
    ; However we are still in 16 bit mode so this will only work if the kernel is smaller than 64KB - a problem for another day
    xor dx, dx
    mov bx, 512
    div bx ; ax = ax / 512
    add ax, 1; read one extra sector to be safe as there may be a partially filled sector

    ; Hacking this to load 128KB

; Now we must load the kernel file from the disk. We add the known start-of-data location to the cluster number.
; We use the size to get the number of sectors to read.

    mov ax, 128
    mov bx, word [cluster]       ; Convert cluster number to logical sector
    add bx, DATA_START
    add bx, -2 ; don't know why yet, but it's at 3295 not 3297

    mov si, 0
    mov cx, KERNEL_SEGMENT       ; put it in the KERNEL_SEGMENT location
    call read_sectors            ; call the function

    mov ax, 128
    mov bx, word [cluster]       ; Convert cluster number to logical sector
    add bx, 128
    add bx, DATA_START
    add bx, -2 ; don't know why yet, but it's at 3295 not 3297

    mov si, 0
    mov cx, KERNEL_SEGMENT       ; put it in the KERNEL_SEGMENT location
    add cx, 0x1000
    call read_sectors            ; call the function
