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
    mov $'\n', %ebx
    mov $1, %eax
    int $0x80
    mov $0, %eax
    int $0x80
