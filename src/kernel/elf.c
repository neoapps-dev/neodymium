#include "elf.h"
#include "vmm.h"
#include "printf.h"
int elf_load(void *elf, unsigned int *entry, unsigned int *stack_top) {
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)elf;
    if (*(unsigned int *)ehdr->e_ident != ELF_MAGIC) {
        printf("[elf] bad magic\n");
        return -1;
    }
    *entry = ehdr->e_entry;
    Elf32_Phdr *phdr = (Elf32_Phdr *)((unsigned char *)elf + ehdr->e_phoff);
    for (unsigned int i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type != PT_LOAD) continue;
        unsigned int vaddr = phdr[i].p_vaddr;
        unsigned int memsz = phdr[i].p_memsz;
        unsigned int filesz = phdr[i].p_filesz;
        unsigned int offset = phdr[i].p_offset;
        unsigned int start = vaddr & ~0xFFF;
        unsigned int end = (vaddr + memsz + 0xFFF) & ~0xFFF;
        for (unsigned int page = start; page < end; page += 0x1000) {
            if (!vmm_alloc_page((void *)page, VMM_USER | VMM_WRITABLE)) {
                printf("[elf] failed to map page 0x%x\n", page);
                return -1;
            }
            for (unsigned int j = 0; j < 0x1000 / 4; j++) ((unsigned int *)page)[j] = 0;
        }
        unsigned char *src = (unsigned char *)elf + offset;
        unsigned char *dst = (unsigned char *)vaddr;
        for (unsigned int j = 0; j < filesz; j++) dst[j] = src[j];
        for (unsigned int j = filesz; j < memsz; j++) dst[j] = 0;
    }
    unsigned int *stack = vmm_alloc_page((void *)0xB0000000, VMM_USER | VMM_WRITABLE);
    if (!stack) return -1;
    for (unsigned int i = 0; i < 0x1000 / 4; i++) stack[i] = 0;
    *stack_top = 0xB0001000;
    return 0;
}
