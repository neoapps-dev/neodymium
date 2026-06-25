#include "kernel.h"
#include "idt.h"
#include "../drivers/ps2.h"
#include "asm/cpu.h"
void kernel_main(void) {
    vga_init();
    serial_init();
    idt_init();
    ps2_init();
    vga_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
    printf("neodymium\n");
    printf("type something:\n");
    vga_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    sti();
    while (1) {
        int c = ps2_getchar();
        if (c == '\b')
            printf("\b \b");
        else if (c >= KEY_F1 && c <= KEY_F12)
            printf("[F%d]", c - KEY_F1 + 1);
        else if (c == KEY_UP) vga_cursor_up();
        else if (c == KEY_DOWN) vga_cursor_down();
        else if (c == KEY_LEFT) vga_cursor_left();
        else if (c == KEY_RIGHT) vga_cursor_right();
        else if (c < 256)
            printf("%c", c);
    }
}
