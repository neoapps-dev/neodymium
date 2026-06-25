#ifndef VGA_H
#define VGA_H
enum vga_color {
    VGA_BLACK = 0,
    VGA_BLUE = 1,
    VGA_GREEN = 2,
    VGA_CYAN = 3,
    VGA_RED = 4,
    VGA_MAGENTA = 5,
    VGA_BROWN = 6,
    VGA_LIGHT_GREY = 7,
    VGA_DARK_GREY = 8,
    VGA_LIGHT_BLUE = 9,
    VGA_LIGHT_GREEN = 10,
    VGA_LIGHT_CYAN = 11,
    VGA_LIGHT_RED = 12,
    VGA_LIGHT_MAGENTA = 13,
    VGA_LIGHT_BROWN = 14,
    VGA_WHITE = 15,
};

static inline unsigned char vga_entry_color(enum vga_color fg, enum vga_color bg) {
    return fg | bg << 4;
}

static inline unsigned short vga_entry(unsigned char uc, unsigned char color) {
    return (unsigned short) uc | (unsigned short) color << 8;
}

void vga_init(void);
void vga_putchar(char c);
void vga_write(const char *data);
void vga_setcolor(unsigned char color);
void vga_cursor_up(void);
void vga_cursor_down(void);
void vga_cursor_left(void);
void vga_cursor_right(void);
#endif
