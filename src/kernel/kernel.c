#include "kernel.h"
#include "idt.h"
#include "pmm.h"
#include "vmm.h"
#include "heap.h"
#include "sched.h"
#include "../boot/multiboot.h"
#include "../drivers/ps2.h"
extern struct multiboot_info *mboot_info;
#include "../drivers/framebuffer.h"
#include "asm/cpu.h"
//static void task_a(void) { while (1) printf("A"); }
//static void task_b(void) { while (1) printf("B"); }
//static void task_c(void) { while (1) printf("C"); }
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

    heap_init();
    unsigned int *a = malloc(8 * sizeof(unsigned int));
    unsigned int *b = malloc(8 * sizeof(unsigned int));
    if (a && b) {
        printf("[heap] a=0x%x b=0x%x diff=%d\n", (unsigned int)a, (unsigned int)b,
               (unsigned int)b - (unsigned int)a);
        a[0] = 0xAAAA;
        b[0] = 0xBBBB;
        printf("[heap] *a=0x%x *b=0x%x\n", a[0], b[0]);
    }
    free(a);
    unsigned int *c = malloc(8 * sizeof(unsigned int));
    if (c) {
        printf("[heap] c=0x%x (reused a? %s)\n", (unsigned int)c,
               (unsigned int)c == (unsigned int)a ? "yes" : "no");
        c[0] = 0xCCCC;
        printf("[heap] *c=0x%x\n", c[0]);
    }
    free(b);
    free(c);
    if (fb_init(mboot_info) == 0) {
        unsigned int bg = fb_rgb(0, 0, 0);
        unsigned int w = fb_get_width();
        unsigned int h = fb_get_height();
        fb_clear(bg);
        for (unsigned int x = 0; x < w; x++) for (unsigned int y = 0; y < 72; y++) if ((x ^ y) & 3) fb_putpixel(x, y, fb_rgb(10 + y / 5, 8 + y / 6, 18 + y / 3));
        fb_drawstring(20, 8, "neodymium framebuffer yay", fb_rgb(255, 200, 70), bg);
        fb_drawstring(20, 28, "x86", fb_rgb(130, 130, 170), bg);
        fb_drawstring(20, 48, MAKE_COMMIT_HASH, fb_rgb(80, 200, 80), bg);
        fb_drawstring(w - 180, 8, "1280x720", fb_rgb(150, 150, 200), bg);
        fb_drawstring(w - 180, 28, "32bpp", fb_rgb(150, 150, 200), bg);
        fb_fillrect(0, 74, w, 2, fb_rgb(40, 30, 60));
        unsigned int swatches[] = {
            fb_rgb(255, 80, 80),   fb_rgb(255, 180, 50),
            fb_rgb(200, 200, 50),  fb_rgb(80, 200, 80),
            fb_rgb(60, 140, 255),  fb_rgb(180, 80, 255),
            fb_rgb(255, 80, 180),
        };
        for (int i = 0; i < 7; i++) fb_fillrect(20 + i * 175, 90, 100, 100, swatches[i]);
        fb_drawstring(20, 210, "pmm  vmm  heap  sched  fb", fb_rgb(120, 120, 160), bg);
        for (int i = 0; i < 5; i++) fb_drawchar(20 + i * 67, 230, "-\\|/-"[(i + h) & 3], fb_rgb(80, 200, 80), bg);
        fb_drawstring(20, 270, "memory:", fb_rgb(150, 150, 180), bg);
        fb_fillrect(90, 272, 600, 12, fb_rgb(20, 20, 30));
        for (int i = 0; i < 54; i++) fb_fillrect(92 + i * 11, 274, 9, 8, fb_rgb(0, 220 - i * 4, 40 + i * 2));
        fb_drawstring(20, 310, "i have no idea what im doing", fb_rgb(180, 160, 120), bg);
        for (unsigned int x = 0; x < w; x++) for (unsigned int y = h - 26; y < h; y++) if ((x + y) & 1) fb_putpixel(x, y, fb_rgb(12, 12, 20));
        fb_drawstring(20, h - 20, "ps/2 ok  serial ok  send help", fb_rgb(100, 100, 120), bg);
        printf("[fb] initialised\n");
    }

    //sched_init();
    //task_create(task_a);
    //task_create(task_b);
    //task_create(task_c);
    //irq_install_handler(0, sched_tick);
    //printf("[sched] 3 tasks running (A, B, C)\n\n");
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
