#ifndef SYSCALL_H
#define SYSCALL_H
//neo: syscall convention is eax=sysno, ebx=arg1, ecx=arg2, edx=arg3, esi=arg4, edi=arg5
#define SYS_EXIT 0
#define SYS_WRITE 1
#define SYS_WRITESTR 2
#define SYS_GETTICK 3
#define SYS_READ 4
#define SYS_FORK 5
#define SYS_GETPID 6
#define SYS_WAIT 7
#define SYS_SLEEP 8
#define SYS_EXEC 9
#include "idt.h"
void syscall_handler(struct regs *r);
void syscall_init(void);
static inline void sys_exit(int code) {
    __asm__ volatile("int $0x80" : : "a"(SYS_EXIT), "b"((unsigned int)code) : "memory");
}
static inline void sys_write(char c) {
    __asm__ volatile("int $0x80" : : "a"(SYS_WRITE), "b"((unsigned int)c) : "memory");
}
static inline void sys_writestr(const char *s) {
    __asm__ volatile("int $0x80" : : "a"(SYS_WRITESTR), "b"(s) : "memory");
}
static inline unsigned int sys_gettick(void) {
    unsigned int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETTICK) : "memory");
    return ret;
}
static inline int sys_read(void) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_READ) : "memory");
    return ret;
}
static inline int sys_fork(void) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_FORK) : "memory");
    return ret;
}
static inline int sys_getpid(void) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_GETPID) : "memory");
    return ret;
}
static inline int sys_wait(void) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_WAIT) : "memory");
    return ret;
}
static inline void sys_sleep(unsigned int ticks) {
    __asm__ volatile("int $0x80" : : "a"(SYS_SLEEP), "b"(ticks) : "memory");
}
static inline int sys_exec(void *elf) {
    int ret;
    __asm__ volatile("int $0x80" : "=a"(ret) : "a"(SYS_EXEC), "b"(elf) : "memory");
    return ret;
}
#endif
