#include "../kernel/asm/boot.h"
__attribute__((section(".multiboot")))
struct {
    unsigned int magic;
    unsigned int flags;
    unsigned int checksum;
} multiboot_header __attribute__((used)) = {
    .magic    = 0x1BADB002,
    .flags    = 0x00000003,
    .checksum = -(0x1BADB002 + 0x00000003)
};

extern void kernel_main(void);
extern char _stack_top[];
__attribute__((section(".text")))
__attribute__((naked))
__attribute__((noreturn))
void _start(void) {
    boot_jump(_stack_top, kernel_main);
}
