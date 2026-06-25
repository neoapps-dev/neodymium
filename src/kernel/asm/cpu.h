#ifndef ASM_CPU_H
#define ASM_CPU_H
struct idtr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));

static inline void sti(void) {
    __asm__ volatile("sti");
}

static inline void cli(void) {
    __asm__ volatile("cli");
}

static inline void hlt(void) {
    __asm__ volatile("hlt");
}

static inline void lidt(struct idtr *idtr) {
    __asm__ volatile("lidt %0" : : "m"(*idtr));
}

#endif
