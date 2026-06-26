#ifndef SCHED_H
#define SCHED_H
#include "idt.h"
void sched_init(void);
int task_create(void (*entry)(void));
int task_create_elf(unsigned int entry, unsigned int user_esp);
void sched_tick(struct regs *r);
int task_create_fork(struct regs *r);
void sched_exit_with(struct regs *r, int code);
void sched_sleep(struct regs *r, unsigned int ticks);
int sched_wait(void);
int sched_getpid(void);
int sched_exec(struct regs *r, void *elf);
unsigned int get_tick(void);
#endif
