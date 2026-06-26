#include "vmm.h"
#include "pmm.h"
#include "printf.h"
#include "idt.h"
#include "asm/cpu.h"
static unsigned int *page_directory;
static void page_fault_handler(struct regs *r) {
    unsigned int fault_addr;
    __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
    printf("[neodymium] PAGE FAULT at 0x%x, eip=0x%x, err=%u\n", fault_addr, r->eip, r->err_code);
    for (;;) hlt();
}

static void vmm_enable(void) {
    unsigned int pd_phys = (unsigned int)page_directory;
    __asm__ volatile(
        "mov %0, %%cr3\n"
        "mov %%cr0, %%eax\n"
        "or $0x80000000, %%eax\n"
        "mov %%eax, %%cr0\n"
        : : "r"(pd_phys) : "eax"
    );
}

void vmm_init(void) {
    unsigned int total_p = pmm_get_total_page_count();
    page_directory = pmm_alloc_page();
    if (!page_directory) {
        printf("[vmm] failed to allocate PD\n");
        for (;;) hlt();
    }

    for (int i = 0; i < 1024; i++) page_directory[i] = 0;
    for (unsigned int i = 0; i < total_p; i += 1024) {
        unsigned int pd_idx = i / 1024;
        unsigned int *pt = pmm_alloc_page();
        if (!pt) {
            printf("[vmm] failed to allocate PT for idx %u\n", pd_idx);
            for (;;) hlt();
        }
        for (unsigned int j = 0; j < 1024 && (i + j) < total_p; j++) pt[j] = ((i + j) * PAGE_SIZE) | VMM_PRESENT | VMM_WRITABLE | VMM_USER;
        page_directory[pd_idx] = ((unsigned int)pt) | VMM_PRESENT | VMM_WRITABLE | VMM_USER;
    }

    exception_install_handler(14, page_fault_handler);
    vmm_enable();
}

void vmm_map_page(void *virt, void *phys, unsigned int flags) {
    unsigned int v = (unsigned int)virt;
    unsigned int p = (unsigned int)phys;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (!(page_directory[pd_idx] & VMM_PRESENT)) {
        unsigned int *pt = pmm_alloc_page();
        if (!pt) return;
        for (unsigned int j = 0; j < 1024; j++)
            pt[j] = 0;
        page_directory[pd_idx] = ((unsigned int)pt) | VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
    }

    unsigned int *pt = (unsigned int *)(page_directory[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = (p & 0xFFFFF000) | flags | VMM_PRESENT;
}

void vmm_unmap_page(void *virt) {
    unsigned int v = (unsigned int)virt;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (!(page_directory[pd_idx] & VMM_PRESENT))return;
    unsigned int *pt = (unsigned int *)(page_directory[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = 0;
    __asm__ volatile("invlpg (%0)" : : "r"(v) : "memory");
}

void *vmm_alloc_page(void *virt, unsigned int flags) {
    void *phys = pmm_alloc_page();
    if (!phys)
        return 0;
    vmm_map_page(virt, phys, flags);
    return phys;
}

void *vmm_get_phys(void *virt) {
    unsigned int v = (unsigned int)virt;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (!(page_directory[pd_idx] & VMM_PRESENT)) return 0;
    unsigned int *pt = (unsigned int *)(page_directory[pd_idx] & 0xFFFFF000);
    if (!(pt[pt_idx] & VMM_PRESENT)) return 0;
    return (void *)((pt[pt_idx] & 0xFFFFF000) + (v & 0xFFF));
}
