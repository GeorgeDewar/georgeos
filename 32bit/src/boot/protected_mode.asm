
; Switch to protected mode
switch_prot:
    cli

    ; Enable A20 line
    ; in al, 0x92
    ; or al, 2
    ; out 0x92, al

    lgdt [gdt_descriptor]
    mov eax, cr0 ; To make the switch to protected mode , we set
    or eax, 0x1 ; the first bit of CR0 , a control register
    mov cr0, eax ; Update the control register

    jmp CODE_SEG:start_protected_mode
    nop
    nop

[bits 32]

    MSG_PROT_MODE       db "Successfully landed in 32-bit Protected Mode", 0

start_protected_mode:
    mov ax, DATA_SEG ; Now in PM , our old segments are meaningless ,
    mov ds, ax ; so we point our segment registers to the
    mov ss, ax ; data selector we defined in our GDT
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ebp, 0x2ffff   ; Update our stack position so it is right
    mov esp, ebp       ; at the top of the free space.

