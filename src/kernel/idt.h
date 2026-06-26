#ifndef IDT_H
#define IDT_H
#define GDT_CODE_SEG 0x08
#define GDT_DATA_SEG 0x10
struct idt_entry {
    unsigned short base_lo;
    unsigned short sel;
    unsigned char always0;
    unsigned char flags;
    unsigned short base_hi;
} __attribute__((packed));
struct regs {
    unsigned int gs, fs, es, ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code;
    unsigned int eip, cs, eflags;
};

void idt_init(void);
void irq_install_handler(int irq, void (*handler)(struct regs *));
void exception_install_handler(int num, void (*handler)(struct regs *));
unsigned int get_tick(void);
#endif
