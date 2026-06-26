#ifndef ELF_H
#define ELF_H
#define ELF_MAGIC 0x464C457F
#define PT_NULL 0
#define PT_LOAD 1
#define PF_X 1
#define PF_W 2
#define PF_R 4
typedef struct {
    unsigned char e_ident[16];
    unsigned short e_type;
    unsigned short e_machine;
    unsigned int e_version;
    unsigned int e_entry;
    unsigned int e_phoff;
    unsigned int e_shoff;
    unsigned int e_flags;
    unsigned short e_ehsize;
    unsigned short e_phentsize;
    unsigned short e_phnum;
    unsigned short e_shentsize;
    unsigned short e_shnum;
    unsigned short e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;
typedef struct {
    unsigned int p_type;
    unsigned int p_offset;
    unsigned int p_vaddr;
    unsigned int p_paddr;
    unsigned int p_filesz;
    unsigned int p_memsz;
    unsigned int p_flags;
    unsigned int p_align;
} __attribute__((packed)) Elf32_Phdr;
int elf_load(void *elf, unsigned int *entry, unsigned int *stack_top);
#endif
