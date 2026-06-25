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
    __asm__ volatile (
        "movl %0, %%esp\n"
        "call kernel_main\n"
        "1: hlt\n"
        "jmp 1b\n"
        :
        : "r" (_stack_top)
        : "memory"
    );
}
