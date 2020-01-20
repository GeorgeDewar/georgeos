global runApplication_

runApplication_:
    push ds
    push es
    call 0x4000:0x0000
    pop es
    pop ds

    ret
