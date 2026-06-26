.section .text
.globl _start
_start:
    mov $'e', %ebx
    mov $1, %eax
    int $0x80
    mov $'x', %ebx
    mov $1, %eax
    int $0x80
    mov $'e', %ebx
    mov $1, %eax
    int $0x80
    mov $'c', %ebx
    mov $1, %eax
    int $0x80
    mov $'!', %ebx
    mov $1, %eax
    int $0x80
    mov $'\n', %ebx
    mov $1, %eax
    int $0x80
    mov $0, %ebx
    mov $0, %eax
    int $0x80
