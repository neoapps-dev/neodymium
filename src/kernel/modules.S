#neo: dont ever touch this. it sucks and i know it. i will get rid of it sometime, maybe when i get ATA and AHCI
.section .note.GNU-stack, "", @progbits
.section .rodata.embedded, "a", @progbits
.globl _embedded_hello_start, _embedded_hello_end
_embedded_hello_start:
    .incbin "build/programs/hello_elf"
_embedded_hello_end:
.globl _embedded_exec_target_start, _embedded_exec_target_end
_embedded_exec_target_start:
    .incbin "build/programs/exec_target_elf"
_embedded_exec_target_end:
