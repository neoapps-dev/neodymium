#ifndef SCHED_H
#define SCHED_H
#include "idt.h"
void sched_init(void);
int task_create(void (*entry)(void));
void sched_tick(struct regs *r);
#endif
