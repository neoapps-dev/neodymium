#include "../kernel/asm/boot.h"
#include "../boot/multiboot.h"
struct multiboot_info *mboot_info;
struct multiboot_info boot_info;
__attribute__((section(".multiboot2")))
struct {
    uint32_t magic;
    uint32_t arch;
    uint32_t len;
    int32_t chk;
    uint16_t fb_type;
    uint16_t fb_flags;
    uint32_t fb_size;
    uint32_t fb_width;
    uint32_t fb_height;
    uint32_t fb_depth;
    uint32_t pad;
    uint16_t end_type;
    uint16_t end_flags;
    uint32_t end_size;
} mb2_header __attribute__((used)) = {
    .magic = MULTIBOOT2_MAGIC,
    .arch = MULTIBOOT2_ARCH_X86,
    .len = 48,
    .chk = -(int32_t)(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH_X86 + 48),
    .fb_type = 5,
    .fb_flags = 0,
    .fb_size = 20,
    .fb_width = 1280,
    .fb_height = 720,
    .fb_depth = 32,
    .pad = 0,
    .end_type = 0,
    .end_flags = 0,
    .end_size = 8,
};

unsigned int module_start;
unsigned int module_end;
extern void kernel_main(void);
extern char _stack_top[];
__attribute__((noreturn))
void _start(void) {
    unsigned int mod_count = 0;
    struct mb2_info *info;
    __asm__ volatile("movl %%ebx, %0" : "=r"(info));
    mboot_info = &boot_info;
    unsigned int pos = sizeof(struct mb2_info);
    while (pos < info->total_size) {
        struct mb2_tag *tag = (struct mb2_tag *)((unsigned int)info + pos);
        if (tag->type == 0) break;
        if (tag->type == 1) {
            boot_info.flags |= (1 << 2);
            boot_info.cmdline = (uint32_t)(tag + 1);
        } else if (tag->type == 4) {
            uint32_t *p = (uint32_t *)((unsigned int)tag + 8);
            boot_info.flags |= (1 << 0);
            boot_info.mem_lower = p[0];
            boot_info.mem_upper = p[1];
        } else if (tag->type == 6) {
            struct mb2_tag_mmap *mt = (void *)tag;
            boot_info.flags |= (1 << 6);
            boot_info.mmap_addr = (uint32_t)(mt + 1);
            boot_info.mmap_length = tag->size - sizeof(struct mb2_tag_mmap);
            boot_info.mmap_entry_size = mt->entry_size;
        } else if (tag->type == 3 && mod_count == 0) {
            uint32_t *p = (uint32_t *)((unsigned int)tag + 8);
            module_start = p[0];
            module_end = p[1];
            mod_count++;
        } else if (tag->type == 8) {
            struct mb2_tag_fb *ft = (void *)tag;
            boot_info.flags |= (1 << 12);
            boot_info.framebuffer_addr = ft->fb_addr;
            boot_info.framebuffer_pitch = ft->fb_pitch;
            boot_info.framebuffer_width = ft->fb_width;
            boot_info.framebuffer_height = ft->fb_height;
            boot_info.framebuffer_bpp = ft->fb_bpp;
            boot_info.framebuffer_type = ft->fb_type;
        } else if (tag->type == 17 || tag->type == 18) {
            boot_info.flags |= MBOOT_FLAG_EFI;
        }
        pos += (tag->size + 7) & ~7;
    }
    boot_jump(_stack_top, kernel_main);
}
