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
static struct gdt_entry gdt[3];
static struct gdtr gdtr;
static void gdt_set(int num, unsigned int base, unsigned int limit, unsigned char access, unsigned char flags) {
    gdt[num].limit_lo = limit & 0xFFFF;
    gdt[num].base_lo = base & 0xFFFF;
    gdt[num].base_mid = (base >> 16) & 0xFF;
    gdt[num].access = access;
    gdt[num].flags_limit_hi = ((limit >> 16) & 0x0F) | (flags & 0xF0);
    gdt[num].base_hi = (base >> 24) & 0xFF;
}

void gdt_init(void) {
    gdtr.limit = sizeof(gdt) - 1;
    gdtr.base = (unsigned int)&gdt;
    gdt_set(0, 0, 0, 0, 0);
    gdt_set(1, 0, 0xFFFFF, 0x9A, 0xCF);
    gdt_set(2, 0, 0xFFFFF, 0x92, 0xCF);
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
}
