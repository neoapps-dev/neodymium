#include "../drivers/vga.h"
#include "kernel.h"
void kernel_main(void) {
    vga_init();
    vga_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
    vga_write("neodymium");
}
