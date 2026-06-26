#include "sched.h"
#include "heap.h"
#include "pmm.h"
#include "printf.h"
#include "asm/cpu.h"
#include "asm/io.h"
#define STACK_SIZE 4096
extern void restore_context(struct regs *r);
struct task {
    struct regs *stack;
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

    struct regs *r = (struct regs *)(stack + STACK_SIZE - sizeof(struct regs));
    for (unsigned int i = 0; i < sizeof(struct regs) / 4; i++) ((unsigned int *)r)[i] = 0;
    r->gs = GDT_DATA_SEG;
    r->fs = GDT_DATA_SEG;
    r->es = GDT_DATA_SEG;
    r->ds = GDT_DATA_SEG;
    r->eip = (unsigned int)entry;
    r->cs = GDT_CODE_SEG;
    r->eflags = 0x202;
    t->stack = r;
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
        idle->next = task_list->next;
        task_list->next = idle;
        current_task = idle;
        return;
    }

    current_task->stack = r;
    current_task = current_task->next;
    outb(0x20, 0x20);
    restore_context(current_task->stack);
}
