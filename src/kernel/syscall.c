#include "syscall.h"
#include "printf.h"
#include "sched.h"
#include "../drivers/ps2.h"
#include "asm/cpu.h"
static void handle_exit(struct regs *r) {
    printf("[sys] exit %d\n", (int)r->ebx);
    sched_exit_with(r, (int)r->ebx);
}
static void handle_write(struct regs *r) {
    if (r->ebx < 256)printf("%c", (char)r->ebx);
}
static void handle_writestr(struct regs *r) {
    if (r->ecx) printf("%s", (const char *)r->ecx);
}
static void handle_gettick(struct regs *r) {
    r->eax = get_tick();
}
static void handle_read(struct regs *r) {
    int c;
    while ((c = ps2_poll()) < 0) hlt();
    r->eax = (unsigned int)c;
}
static void handle_fork(struct regs *r) {
    if (task_create_fork(r) == 0)
        r->eax = 1;
    else
        r->eax = (unsigned int)-1;
}
static void handle_getpid(struct regs *r) {
    r->eax = (unsigned int)sched_getpid();
}
static void handle_wait(struct regs *r) {
    int ret = sched_wait();
    if (ret >= 0) printf("[sys] wait -> %d\n", ret);
    r->eax = (unsigned int)ret;
}
static void handle_sleep(struct regs *r) {
    r->eax = 0;
    sched_sleep(r, r->ebx);
}
static void handle_exec(struct regs *r) {
    if (sched_exec(r, (void *)r->ebx) != 0) r->eax = (unsigned int)-1;
}
void syscall_handler(struct regs *r) {
    switch (r->eax) {
        case SYS_EXIT:     handle_exit(r);     break;
        case SYS_WRITE:    handle_write(r);    break;
        case SYS_WRITESTR: handle_writestr(r); break;
        case SYS_GETTICK:  handle_gettick(r);  break;
        case SYS_READ:     handle_read(r);     break;
        case SYS_FORK:     handle_fork(r);     break;
        case SYS_GETPID:   handle_getpid(r);   break;
        case SYS_WAIT:     handle_wait(r);     break;
        case SYS_SLEEP:    handle_sleep(r);    break;
        case SYS_EXEC:     handle_exec(r);     break;
        default: printf("[sys] unknown: %u\n", r->eax); break;
    }
}
void syscall_init(void) {
}
