#include "sched.h"
#include "gdt.h"
#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "printf.h"
#include "asm/cpu.h"
#include "asm/io.h"
#define STACK_SIZE 4096
extern void restore_context(struct regs *r);
struct task {
    struct regs *stack;
    unsigned int stack_top;
    struct task *next;
};

static struct task *task_list;
static struct task *current_task;
static volatile unsigned int tick;
void sched_init(void) {
    task_list = 0;
    current_task = 0;
    tick = 0;
}

int task_create(void (*entry)(void)) {
    struct task *t = malloc(sizeof(struct task));
    if (!t) return -1;
    unsigned char *stack = malloc(STACK_SIZE);
    if (!stack) {
        free(t);
        return -1;
    }

    unsigned char *user_stack = vmm_alloc_page((void *)0xB0000000, VMM_USER | VMM_WRITABLE);
    if (!user_stack) {
        free(stack);
        free(t);
        return -1;
    }
    for (unsigned int i = 0; i < PAGE_SIZE / 4; i++) ((unsigned int *)0xB0000000)[i] = 0;
    struct regs *r = (struct regs *)(stack + STACK_SIZE - sizeof(struct regs));
    for (unsigned int i = 0; i < sizeof(struct regs) / 4; i++) ((unsigned int *)r)[i] = 0;
    r->gs = GDT_USER_DATA_SEG | 3;
    r->fs = GDT_USER_DATA_SEG | 3;
    r->es = GDT_USER_DATA_SEG | 3;
    r->ds = GDT_USER_DATA_SEG | 3;
    r->eip = (unsigned int)entry;
    r->cs = GDT_USER_CODE_SEG | 3;
    r->eflags = 0x202;
    r->user_esp = 0xB0001000;
    r->user_ss = GDT_USER_DATA_SEG | 3;
    t->stack = r;
    t->stack_top = (unsigned int)stack + STACK_SIZE;
    if (!task_list) {
        task_list = t;
        t->next = t;
    } else {
        t->next = task_list->next;
        task_list->next = t;
    }

    return 0;
}

int task_create_elf(unsigned int entry, unsigned int user_esp) {
    struct task *t = malloc(sizeof(struct task));
    if (!t) return -1;
    unsigned char *stack = malloc(STACK_SIZE);
    if (!stack) {
        free(t);
        return -1;
    }

    struct regs *r = (struct regs *)(stack + STACK_SIZE - sizeof(struct regs));
    for (unsigned int i = 0; i < sizeof(struct regs) / 4; i++) ((unsigned int *)r)[i] = 0;
    r->gs = GDT_USER_DATA_SEG | 3;
    r->fs = GDT_USER_DATA_SEG | 3;
    r->es = GDT_USER_DATA_SEG | 3;
    r->ds = GDT_USER_DATA_SEG | 3;
    r->eip = entry;
    r->cs = GDT_USER_CODE_SEG | 3;
    r->eflags = 0x202;
    r->user_esp = user_esp;
    r->user_ss = GDT_USER_DATA_SEG | 3;
    t->stack = r;
    t->stack_top = (unsigned int)stack + STACK_SIZE;
    if (!task_list) {
        task_list = t;
        t->next = t;
    } else {
        t->next = task_list->next;
        task_list->next = t;
    }
    return 0;
}

void sched_tick(struct regs *r) {
    tick++;
    if (!task_list) return;
    if (!current_task) {
        struct task *idle = malloc(sizeof(struct task));
        if (!idle) return;
        idle->stack = r;
        idle->stack_top = 0;
        idle->next = task_list->next;
        task_list->next = idle;
        current_task = idle;
        return;
    }

    current_task->stack = r;
    current_task = current_task->next;
    tss_set_esp0(current_task->stack_top);
    outb(0x20, 0x20);
    restore_context(current_task->stack);
}

void task_exit(void) {
    if (!task_list || !current_task) for (;;) hlt();
    struct task *t = task_list;
    while (t->next != current_task) t = t->next;
    struct task *dead = current_task;
    t->next = dead->next;
    if (dead == t && dead->next == dead) {
        task_list = 0;
        current_task = 0;
        for (;;) hlt();
    }
    current_task = t->next;
    tss_set_esp0(current_task->stack_top);
    if (dead->stack_top) free((void *)(dead->stack_top - STACK_SIZE));
    free(dead);
    restore_context(current_task->stack);
}
