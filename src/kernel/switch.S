.section .text
.global restore_context
restore_context:
    mov 4(%esp), %eax
    mov %eax, %esp
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popal
    add $8, %esp
    iret

.section .note.GNU-stack,"",@progbits
