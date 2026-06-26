#include "kernel.h"
#include "idt.h"
#include "pmm.h"
#include "vmm.h"
#include "../drivers/ps2.h"
#include "asm/cpu.h"
extern struct multiboot_info *mboot_info;
void kernel_main(void) {
    vga_init();
    serial_init();
    idt_init();
    pmm_init(mboot_info);
    ps2_init();
    vga_setcolor(vga_entry_color(VGA_WHITE, VGA_BLACK));
    printf("//neodymium [%s]\n\n", MAKE_COMMIT_HASH);
    unsigned int free_pages = pmm_get_free_page_count();
    printf("[pmm] %u KiB free\n", free_pages * 4);
    void *page = pmm_alloc_page();
    if (page) printf("[pmm] allocated page at 0x%x\n", (unsigned int)page);
    pmm_free_page(page);
    vmm_init();
    void *vpage = vmm_alloc_page((void *)0xE0000000, VMM_WRITABLE);
    if (vpage) {
        *(unsigned int *)0xE0000000 = 0xDEADBEEF;
        printf("[vmm] 0xE0000000 = 0x%x\n", *(unsigned int *)0xE0000000);
        printf("[vmm] phys = 0x%x\n", (unsigned int)vmm_get_phys((void *)0xE0000000));
        vmm_unmap_page((void *)0xE0000000);
        pmm_free_page(vpage);
    }

    sti();
    printf("[ps/2] type something:\n\n\n");
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
