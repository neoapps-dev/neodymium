#include "../kernel/asm/boot.h"
#include "../boot/multiboot.h"
struct multiboot_info *mboot_info;
__attribute__((section(".multiboot")))
struct {
    unsigned int magic;
    unsigned int flags;
    unsigned int checksum;
    unsigned int mode_type;
    unsigned int width;
    unsigned int height;
    unsigned int depth;
} multiboot_header __attribute__((used)) = {
    .magic    = 0x1BADB002,
    .flags    = 0x00000007,
    .checksum = -(0x1BADB002 + 0x00000007),
    .mode_type = 0,
    .width     = 1280,
    .height    = 720,
    .depth     = 32
};

extern void kernel_main(void);
extern char _stack_top[];
__attribute__((section(".text")))
__attribute__((naked))
__attribute__((noreturn))
void _start(void) {
    __asm__ volatile("movl %%ebx, %0" : "=m"(mboot_info));
    boot_jump(_stack_top, kernel_main);
}
