.section .text
.globl _start
_start:
    mov $'H', %ebx
    mov $1, %eax
    int $0x80
    mov $'e', %ebx
    mov $1, %eax
    int $0x80
    mov $'l', %ebx
    mov $1, %eax
    int $0x80
    mov $'l', %ebx
    mov $1, %eax
    int $0x80
    mov $'o', %ebx
    mov $1, %eax
    int $0x80
    mov $'!', %ebx
    mov $1, %eax
    int $0x80
    mov $' ', %ebx
    mov $1, %eax
    int $0x80
    mov $6, %eax
    int $0x80
    mov %eax, %ecx
    mov $10, %ebx
    mov $0, %edi
1:  mov $0, %edx
    div %ebx
    push %edx
    inc %edi
    cmpl $0, %eax
    jne 1b
2:  pop %ebx
    add $'0', %ebx
    mov $1, %eax
    int $0x80
    dec %edi
    cmpl $0, %edi
    jne 2b
    mov $'\n', %ebx
    mov $1, %eax
    int $0x80
    mov 0xE0001000, %ebx
    mov $9, %eax
    int $0x80
    mov $'X', %ebx
    mov $1, %eax
    int $0x80
    mov $'\n', %ebx
    mov $1, %eax
    int $0x80
    mov $0, %ebx
    mov $0, %eax
    int $0x80
