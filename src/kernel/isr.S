.altmacro
.section .text
.macro isr_noerr n
.align 4
.global isr_stub_\n
isr_stub_\n:
    push $0
    push $\n
    jmp isr_common
.endm
.macro isr_err n
.align 4
.global isr_stub_\n
isr_stub_\n:
    push $\n
    jmp isr_common
.endm
.macro stub_entry n
.long isr_stub_\n
.endm
isr_noerr 0
isr_noerr 1
isr_noerr 2
isr_noerr 3
isr_noerr 4
isr_noerr 5
isr_noerr 6
isr_noerr 7
isr_err 8
isr_noerr 9
isr_err 10
isr_err 11
isr_err 12
isr_err 13
isr_err 14
isr_noerr 15
isr_noerr 16
isr_err 17
isr_noerr 18
isr_noerr 19
isr_noerr 20
isr_err 21
isr_noerr 22
isr_noerr 23
isr_noerr 24
isr_noerr 25
isr_noerr 26
isr_noerr 27
isr_noerr 28
isr_err 29
isr_err 30
isr_noerr 31
.set i, 32
.rept 224
    isr_noerr %i
    .set i, i + 1
.endr
isr_common:
    pushal
    push %ds
    push %es
    push %fs
    push %gs
    mov %esp, %eax
    push %eax
    call isr_handler
    add $4, %esp
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal
    add $8, %esp
    iret

.global isr_stub_table
isr_stub_table:
    .set i, 0
    .rept 256
        stub_entry %i
        .set i, i + 1
    .endr

.section .note.GNU-stack,"",@progbits
