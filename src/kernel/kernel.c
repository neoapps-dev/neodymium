#include "../drivers/drivers.h"
#include "kernel.h"
void kernel_main(void) {
    vga_init();
    serial_init();
    vga_setcolor(vga_entry_color(VGA_RED, VGA_BLACK));
    vga_write("neodymium\n");
    serial_write("neodymium\n");
}
