#ifndef SYSCALL_H
#define SYSCALL_H
#define SYS_EXIT 0
#define SYS_WRITE 1
#define SYS_WRITESTR 2
#define SYS_GETTICK 3
#include "idt.h"
void syscall_handler(struct regs *r);
void syscall_init(void);
static inline void sys_exit(void) {
    __asm__ volatile("int $0x80" : : "a"(SYS_EXIT) : "memory");
}
static inline void sys_write(char c) {
    __asm__ volatile("int $0x80" : : "a"(SYS_WRITE), "b"((unsigned int)c) : "memory");
}
static inline void sys_writestr(const char *s) {
    __asm__ volatile("int $0x80" : : "a"(SYS_WRITESTR), "c"(s) : "memory");
}
static inline unsigned int sys_gettick(void) {
    unsigned int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETTICK) : "memory");
    return ret;
}
#endif
