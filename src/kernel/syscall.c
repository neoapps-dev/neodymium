#include "syscall.h"
#include "printf.h"
#include "asm/cpu.h"
extern unsigned int get_tick(void);
static void handle_exit(struct regs *r) {
    (void)r;
    printf("[sys] exit\n");
    for (;;) hlt();
}
static void handle_write(struct regs *r) {
    if (r->ebx < 256)
        printf("%c", (char)r->ebx);
}
static void handle_writestr(struct regs *r) {
    if (r->ecx)
        printf("%s", (const char *)r->ecx);
}
static void handle_gettick(struct regs *r) {
    r->eax = get_tick();
}
void syscall_handler(struct regs *r) {
    switch (r->eax) {
        case SYS_EXIT:     handle_exit(r);     break;
        case SYS_WRITE:    handle_write(r);    break;
        case SYS_WRITESTR: handle_writestr(r); break;
        case SYS_GETTICK:  handle_gettick(r);  break;
        default: printf("[sys] unknown: %u\n", r->eax); break;
    }
}
void syscall_init(void) {
}
