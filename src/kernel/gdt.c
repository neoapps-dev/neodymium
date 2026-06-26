#include "gdt.h"
#include "idt.h"
struct gdt_entry {
    unsigned short limit_lo;
    unsigned short base_lo;
    unsigned char base_mid;
    unsigned char access;
    unsigned char flags_limit_hi;
    unsigned char base_hi;
} __attribute__((packed));
struct gdtr {
    unsigned short limit;
    unsigned int base;
} __attribute__((packed));
struct tss_entry {
    unsigned int prev_tss;
    unsigned int esp0;
    unsigned int ss0;
    unsigned int esp1;
    unsigned int ss1;
    unsigned int esp2;
    unsigned int ss2;
    unsigned int cr3;
    unsigned int eip;
    unsigned int eflags;
    unsigned int eax, ecx, edx, ebx;
    unsigned int esp, ebp, esi, edi;
    unsigned int es, cs, ss, ds, fs, gs;
    unsigned int ldt;
    unsigned short trap;
    unsigned short iomap_base;
} __attribute__((packed));
static struct gdt_entry gdt[6];
static struct gdtr gdtr;
static struct tss_entry tss;
static void gdt_set(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags) {
    gdt[num].limit_lo = limit & 0xFFFF;
    gdt[num].base_lo = base & 0xFFFF;
    gdt[num].base_mid = (base >> 16) & 0xFF;
    gdt[num].access = access;
    gdt[num].flags_limit_hi = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    gdt[num].base_hi = (base >> 24) & 0xFF;
}

static void tss_init(void) {
    unsigned int base = (unsigned int)&tss;
    unsigned int limit = sizeof(tss) - 1;
    for (unsigned int i = 0; i < sizeof(tss) / 4; i++) ((unsigned int *)&tss)[i] = 0;
    tss.ss0 = GDT_DATA_SEG;
    gdt_set(5, base, limit, 0x89, 0x00);
}

void tss_set_esp0(unsigned int esp0) {
    tss.esp0 = esp0;
}

void gdt_init(void) {
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (unsigned int)&gdt;
    gdt_set(0, 0, 0, 0, 0);
    gdt_set(1, 0, 0xFFFFF, 0x9A, 0xCF);
    gdt_set(2, 0, 0xFFFFF, 0x92, 0xCF);
    gdt_set(3, 0, 0xFFFFF, 0xFA, 0xCF);
    gdt_set(4, 0, 0xFFFFF, 0xF2, 0xCF);
    tss_init();
    __asm__ volatile("lgdt %0" : : "m"(gdtr));
    __asm__ volatile(
        "mov %0, %%ax\n"
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp %1, $1f\n"
        "1:\n"
        :
        : "i"(GDT_DATA_SEG), "i"(GDT_CODE_SEG)
        : "memory"
    );
    __asm__ volatile("ltr %0" : : "r"((unsigned short)GDT_TSS_SEG));
}
