#ifndef ASM_BOOT_H
#define ASM_BOOT_H
static inline __attribute__((noreturn)) void boot_jump(void *stack, void (*entry)(void)) {
    __asm__ volatile(
        "movl %0, %%esp\n"
        "call *%1\n"
        "1: hlt\n"
        "jmp 1b\n"
        :
        : "r"(stack), "r"(entry)
        : "memory"
    );
    __builtin_unreachable();
}

#endif
