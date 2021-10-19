[bits 32]

%macro make_irq 1
    global irq%1
    irq%1:
        cli
        push byte 0          ; Push a dummy error code to keep a uniform stack frame
        push byte %1 + 32    ; Push the interrupt number
        jmp irq_common_stub
%endmacro

make_irq 0
make_irq 1
make_irq 2
make_irq 3
make_irq 4
make_irq 5
make_irq 6
make_irq 7
make_irq 8
make_irq 9
make_irq 10
make_irq 11
make_irq 12
make_irq 13
make_irq 14
make_irq 15

extern irq_handler

; This is a stub that we have created for IRQ based ISRs. This calls
; '_irq_handler' in our C code. We need to create this in an 'irq.c'
irq_common_stub:
    pusha
    push ss
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp
    push eax
    mov eax, irq_handler
    call eax
    pop eax
    pop gs
    pop fs
    pop es
    pop ds
    pop ss
    popa
    add esp, 8
    iret
