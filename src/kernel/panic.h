#ifndef PANIC_H
#define PANIC_H
#include "idt.h"
void panic(const char *fmt, ...) __attribute__((noreturn));
void panic_regs(const char *fmt, struct regs *r, ...) __attribute__((noreturn));
#endif
