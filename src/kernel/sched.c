#include "sched.h"
#include "gdt.h"
#include "heap.h"
#include "pmm.h"
#include "vmm.h"
#include "elf.h"
#include "printf.h"
#include "asm/cpu.h"
#include "asm/io.h"
#include "../drivers/ps2.h"
#include "../drivers/fbcon.h"
#include "../drivers/framebuffer.h"

#define STACK_SIZE 4096

#define TASK_RUNNING  0
#define TASK_ZOMBIE   1
#define TASK_SLEEPING 2

extern void restore_context(struct regs *r);

struct task {
    struct regs *stack;
    unsigned int stack_top;
    struct task *next;
    struct task *parent;
    int pid;
    int exit_code;
    int state;
    unsigned int wakeup_tick;
};

static struct task *task_list;
static struct task *current_task;
static volatile unsigned int tick;

unsigned int get_tick(void) {
    return tick;
}

static struct task *pick_next_task(void) {
    if (!task_list) return 0;
    struct task *start = current_task ? current_task : task_list;
    struct task *t = start;
    do {
        t = t->next;
        if (t->state == TASK_RUNNING)
            return t;
    } while (t != start);
    return 0;
}

static int next_pid = 1;

void sched_init(void) {
    task_list = 0;
    current_task = 0;
    tick = 0;
    next_pid = 1;
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
    t->pid = next_pid++;
    t->parent = 0;
    t->state = TASK_RUNNING;
    t->exit_code = 0;
    t->wakeup_tick = 0;
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
    t->pid = next_pid++;
    t->parent = 0;
    t->state = TASK_RUNNING;
    t->exit_code = 0;
    t->wakeup_tick = 0;
    if (!task_list) {
        task_list = t;
        t->next = t;
    } else {
        t->next = task_list->next;
        task_list->next = t;
    }
    return 0;
}

int task_create_fork(struct regs *r) {
    struct task *t = malloc(sizeof(struct task));
    if (!t) return -1;
    unsigned char *stack = malloc(STACK_SIZE);
    if (!stack) {
        free(t);
        return -1;
    }
    struct regs *cr = (struct regs *)(stack + STACK_SIZE - sizeof(struct regs));
    *cr = *r;
    cr->eax = 0;
    t->stack = cr;
    t->stack_top = (unsigned int)stack + STACK_SIZE;
    t->pid = next_pid++;
    t->parent = current_task;
    t->state = TASK_RUNNING;
    t->exit_code = 0;
    t->wakeup_tick = 0;
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
    struct task *t = task_list;
    do {
        if (t->state == TASK_SLEEPING && t->wakeup_tick <= tick)
            t->state = TASK_RUNNING;
        t = t->next;
    } while (t != task_list);
    if (current_task && current_task->state == TASK_RUNNING) current_task->stack = r;
    struct task *next = pick_next_task();
    if (next) {
        outb(0x20, 0x20);
        current_task = next;
        tss_set_esp0(current_task->stack_top);
        restore_context(current_task->stack);
    }
}

static void yield(struct regs *r) {
    if (current_task)
        current_task->stack = r;
    sti();
    for (;;) {
        struct task *next = pick_next_task();
        if (next) {
            current_task = next;
            tss_set_esp0(current_task->stack_top);
            restore_context(current_task->stack);
        }
        int c = ps2_poll();
        if (c >= 0) {
            if (c == '\b')
                printf("\b \b");
            else if (c == KEY_F12 && ps2_is_ctrl() && ps2_is_shift() && fb_is_enabled()) {
                if (fbcon_get_visible()) fbcon_set_visible(0);
                else {
                    fbcon_set_visible(1);
                    fb_clear(fb_rgb(0, 0, 0));
                    fbcon_redraw();
                }
            } else if (c >= KEY_F1 && c <= KEY_F12)
                printf("[F%d]", c - KEY_F1 + 1);
            else if (c == KEY_UP)
                fbcon_cursor_up();
            else if (c == KEY_DOWN)
                fbcon_cursor_down();
            else if (c == KEY_LEFT)
                fbcon_cursor_left();
            else if (c == KEY_RIGHT)
                fbcon_cursor_right();
            else if (c < 256)
                printf("%c", c);
        }
        fbcon_tick();
        hlt();
    }
}

void sched_exit_with(struct regs *r, int code) {
    if (!task_list || !current_task)
        for (;;) hlt();
    current_task->exit_code = code;
    current_task->state = TASK_ZOMBIE;
    yield(r);
}

void sched_sleep(struct regs *r, unsigned int ticks) {
    if (!task_list || !current_task) return;
    current_task->wakeup_tick = tick + ticks;
    current_task->state = TASK_SLEEPING;
    yield(r);
}

int sched_wait(void) {
    if (!task_list || !current_task) return -1;
    struct task *t = task_list;
    struct task *start = t;
    do {
        if (t->parent == current_task && t->state == TASK_ZOMBIE) {
            int code = t->exit_code;
            struct task *prev = task_list;
            while (prev->next != t) prev = prev->next;
            prev->next = t->next;
            if (task_list == t)
                task_list = t->next;
            if (task_list == t)
                task_list = 0;
            if (t->stack_top)
                free((void *)(t->stack_top - STACK_SIZE));
            free(t);
            return code;
        }
        t = t->next;
    } while (t != start);
    return -1;
}

int sched_getpid(void) {
    if (current_task) return current_task->pid;
    return -1;
}

int sched_exec(struct regs *r, void *elf) {
    if (!current_task) return -1;
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf;
    if (*(unsigned int *)ehdr->e_ident != ELF_MAGIC)
        return -1;
    unsigned int entry, stack_top;
    if (elf_load(elf, &entry, &stack_top) != 0)
        return -1;
    r->eip = entry;
    r->user_esp = stack_top;
    r->eax = 0;
    return 0;
}
