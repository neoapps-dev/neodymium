SECTIONS {
    . = 0x08048000;
    .text : { *(.text) }
    /DISCARD/ : { *(*) }
}
