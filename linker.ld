ENTRY(_start)
SECTIONS
{
    . = 1M;
    _kernel_start = .;
    .multiboot2 : {
        *(.multiboot2)
    }

    .text : {
        *(.text)
    }

    .rodata : {
        *(.rodata)
    }

    .data : {
        *(.data)
    }

    .bss : {
        *(COMMON)
        *(.bss)
        . = ALIGN(16);
        _stack_bottom = .;
        . += 16K;
        _stack_top = .;
    }

    _kernel_end = .;
    /DISCARD/ : {
        *(.note.gnu.property)
        *(.eh_frame)
        *(.comment)
    }
}
