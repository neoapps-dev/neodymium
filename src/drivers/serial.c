#include "serial.h"
#include "../kernel/asm/io.h"
static const unsigned short COM1 = 0x3F8;
void serial_init(void) {
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x01);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 1, 0x00);
}

void serial_putchar(char c) {
    while (!(inb(COM1 + 5) & 0x20));
    outb(COM1 + 0, (unsigned char)c);
}

void serial_write(const char *data) {
    for (const char *p = data; *p != '\0'; p++) {
        serial_putchar(*p);
    }
}
