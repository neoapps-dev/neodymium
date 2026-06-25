#include "idt.h"
#include "asm/cpu.h"
#include "asm/io.h"
static struct idt_entry idt[256];
static void (*irq_handlers[16])(struct regs *);
extern unsigned int isr_stub_table[];
static void idt_set(int num, void *base, unsigned short sel, unsigned char flags) {
    unsigned int b = (unsigned int)base;
    idt[num].base_lo = b & 0xFFFF;
    idt[num].base_hi = (b >> 16) & 0xFFFF;
    idt[num].sel = sel;
    idt[num].always0 = 0;
    idt[num].flags = flags;
}

static void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20);
    outb(0xA1, 0x28);
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    outb(0x21, 0xFE);
    outb(0xA1, 0xFF);
}

static void pit_init(void) {
    unsigned short divisor = 4773; //neo: 250 hertz
    outb(0x43, 0x36);
    outb(0x40, divisor & 0xFF);
    outb(0x40, divisor >> 8);
}

void irq_install_handler(int irq, void (*handler)(struct regs *)) {
    irq_handlers[irq] = handler;
}

void isr_handler(struct regs *r) {
    if (r->int_no >= 32 && r->int_no <= 47) {
        int irq = r->int_no - 32;
        if (irq_handlers[irq])
            irq_handlers[irq](r);
        if (r->int_no >= 40)
            outb(0xA0, 0x20);
        outb(0x20, 0x20);
    }
}

void idt_init(void) {
    struct idtr idtr;
    idtr.limit = sizeof(idt) - 1;
    idtr.base = (unsigned int)&idt;
    for (int i = 0; i < 256; i++)
        idt_set(i, (void *)isr_stub_table[i], GDT_CODE_SEG, 0x8E);

    pic_remap();
    pit_init();
    lidt(&idtr);
}
