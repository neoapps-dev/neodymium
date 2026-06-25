#include "vga.h"
#include "../kernel/asm/io.h"
static const unsigned int VGA_WIDTH  = 80;
static const unsigned int VGA_HEIGHT = 25;
static unsigned short *vga_buffer;
static unsigned int vga_row;
static unsigned int vga_column;
static unsigned char vga_color;
static void vga_update_cursor(void) {
    unsigned short pos = vga_row * VGA_WIDTH + vga_column;
    outb(0x3D4, 0x0E);
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 0x0F);
    outb(0x3D5, pos & 0xFF);
}
void vga_init(void) {
    vga_buffer = (unsigned short *) 0xB8000;
    vga_row    = 0;
    vga_column = 0;
    vga_color  = vga_entry_color(VGA_LIGHT_GREY, VGA_BLACK);
    for (unsigned int y = 0; y < VGA_HEIGHT; y++) {
        for (unsigned int x = 0; x < VGA_WIDTH; x++) {
            const unsigned int index = y * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
    }
}

void vga_setcolor(unsigned char color) {
    vga_color = color;
}

void vga_putchar(char c) {
    if (c == '\n') {
        vga_column = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_column = 0;
    } else if (c == '\b') {
        if (vga_column > 0) vga_column--;
        else if (vga_row > 0) { vga_row--; vga_column = VGA_WIDTH - 1; }
        const unsigned int index = vga_row * VGA_WIDTH + vga_column;
        vga_buffer[index] = vga_entry(' ', vga_color);
    } else if (c == '\t') {
        vga_column = (vga_column + 4) & ~3;
    } else {
        const unsigned int index = vga_row * VGA_WIDTH + vga_column;
        vga_buffer[index] = vga_entry((unsigned char) c, vga_color);
        vga_column++;
    }

    if (vga_column >= VGA_WIDTH) {
        vga_column = 0;
        vga_row++;
    }

    if (vga_row >= VGA_HEIGHT) {
        for (unsigned int y = 0; y < VGA_HEIGHT - 1; y++) {
            for (unsigned int x = 0; x < VGA_WIDTH; x++) {
                const unsigned int from = (y + 1) * VGA_WIDTH + x;
                const unsigned int to   = y * VGA_WIDTH + x;
                vga_buffer[to] = vga_buffer[from];
            }
        }
        for (unsigned int x = 0; x < VGA_WIDTH; x++) {
            const unsigned int index = (VGA_HEIGHT - 1) * VGA_WIDTH + x;
            vga_buffer[index] = vga_entry(' ', vga_color);
        }
        vga_row = VGA_HEIGHT - 1;
    }
    vga_update_cursor();
}

void vga_cursor_up(void) {
    if (vga_row > 0) vga_row--;
    vga_update_cursor();
}

void vga_cursor_down(void) {
    if (vga_row < VGA_HEIGHT - 1) vga_row++;
    vga_update_cursor();
}

void vga_cursor_left(void) {
    if (vga_column > 0) vga_column--;
    else if (vga_row > 0) { vga_row--; vga_column = VGA_WIDTH - 1; }
    vga_update_cursor();
}

void vga_cursor_right(void) {
    if (vga_column < VGA_WIDTH - 1) vga_column++;
    else if (vga_row < VGA_HEIGHT - 1) { vga_row++; vga_column = 0; }
    vga_update_cursor();
}

void vga_write(const char *data) {
    for (const char *p = data; *p != '\0'; p++) {
        vga_putchar(*p);
    }
}
