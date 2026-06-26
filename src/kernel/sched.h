#ifndef SCHED_H
#define SCHED_H
#include "idt.h"
void sched_init(void);
int task_create(void (*entry)(void));
int task_create_elf(unsigned int entry, unsigned int user_esp);
void sched_tick(struct regs *r);
void task_exit(void);
#endif
