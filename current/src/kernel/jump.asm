global runApplication_
global handleCallAsm

extern handleCall_

runApplication_:
    push ds                     ; Push DS to the stack so we can restore it after execution
    push es                     ; Push ES also
    call 0x4000:0x0000          ; Jump to where we loaded our program
    pop es                      ; Restore ES and DS from the stack
    pop ds                      ;

    ret                         ; Return

handleCallAsm:
    jmp $
    push ds                     ; Push DS to the stack so we can restore it after execution
    push es                     ; Push ES also
    ;call handleCall_            ; Call our main interrupt handler function written in C
    pop es                      ; Restore ES and DS from the stack
    pop ds                      ;

    iret                        ; Return from the interrupt handler
