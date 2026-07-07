#include "vmm.h"
#include "pmm.h"
#include "printf.h"
#include "idt.h"
#include "asm/cpu.h"
static unsigned int *current_page_directory;
static unsigned int *kernel_page_directory;
static unsigned int kernel_pd_count;
static void page_fault_handler(struct regs *r) {
    unsigned int fault_addr;
    __asm__ volatile("mov %%cr2, %0" : "=r"(fault_addr));
    printf("[neodymium] PAGE FAULT at 0x%x, eip=0x%x, err=%u\n", fault_addr, r->eip, r->err_code);
    for (;;) hlt();
}

static void vmm_enable(void) {
    unsigned int pd_phys = (unsigned int)current_page_directory;
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
    kernel_page_directory = pmm_alloc_page();
    if (!kernel_page_directory) {
        printf("[vmm] failed to allocate PD\n");
        for (;;) hlt();
    }
    current_page_directory = kernel_page_directory;
    for (int i = 0; i < 1024; i++) kernel_page_directory[i] = 0;
    for (unsigned int i = 0; i < total_p; i += 1024) {
        unsigned int pd_idx = i / 1024;
        unsigned int *pt = pmm_alloc_page();
        if (!pt) {
            printf("[vmm] failed to allocate PT for idx %u\n", pd_idx);
            for (;;) hlt();
        }
        for (unsigned int j = 0; j < 1024 && (i + j) < total_p; j++) pt[j] = ((i + j) * PAGE_SIZE) | VMM_PRESENT | VMM_WRITABLE | VMM_USER;
        kernel_page_directory[pd_idx] = ((unsigned int)pt) | VMM_PRESENT | VMM_WRITABLE | VMM_USER;
    }

    kernel_pd_count = (total_p + 1023) / 1024;
    exception_install_handler(14, page_fault_handler);
    vmm_enable();
}

void vmm_map_page(void *virt, void *phys, unsigned int flags) {
    unsigned int v = (unsigned int)virt;
    unsigned int p = (unsigned int)phys;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (current_page_directory != kernel_page_directory
        && current_page_directory[pd_idx] & VMM_PRESENT
        && current_page_directory[pd_idx] == kernel_page_directory[pd_idx]) {
        unsigned int *new_pt = pmm_alloc_page();
        if (!new_pt) return;
        unsigned int *old_pt = (unsigned int *)(current_page_directory[pd_idx] & 0xFFFFF000);
        for (unsigned int j = 0; j < 1024; j++)new_pt[j] = old_pt[j];
        current_page_directory[pd_idx] = ((unsigned int)new_pt) | (current_page_directory[pd_idx] & 0xFFF);
        if (flags & VMM_USER) current_page_directory[pd_idx] |= VMM_USER;
    }

    if (!(current_page_directory[pd_idx] & VMM_PRESENT)) {
        unsigned int *pt = pmm_alloc_page();
        if (!pt) return;
        for (unsigned int j = 0; j < 1024; j++)pt[j] = 0;
        current_page_directory[pd_idx] = ((unsigned int)pt) | VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
    }
    if (flags & VMM_USER) current_page_directory[pd_idx] |= VMM_USER;
    unsigned int *pt = (unsigned int *)(current_page_directory[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = (p & 0xFFFFF000) | flags | VMM_PRESENT;
}

void vmm_unmap_page(void *virt) {
    unsigned int v = (unsigned int)virt;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (!(current_page_directory[pd_idx] & VMM_PRESENT))return;
    unsigned int *pt = (unsigned int *)(current_page_directory[pd_idx] & 0xFFFFF000);
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

void vmm_free_page(void *virt) {
    void *phys = vmm_get_phys(virt);
    if (phys) {
        vmm_unmap_page(virt);
        pmm_free_page(phys);
    }
}

void *vmm_get_phys(void *virt) {
    unsigned int v = (unsigned int)virt;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (!(current_page_directory[pd_idx] & VMM_PRESENT)) return 0;
    unsigned int *pt = (unsigned int *)(current_page_directory[pd_idx] & 0xFFFFF000);
    if (!(pt[pt_idx] & VMM_PRESENT)) return 0;
    return (void *)((pt[pt_idx] & 0xFFFFF000) + (v & 0xFFF));
}

unsigned int *vmm_create_address_space(void) {
    unsigned int *pd = pmm_alloc_page();
    if (!pd) return 0;
    for (unsigned int i = 0; i < 1024; i++) {
        if (kernel_page_directory[i] & VMM_PRESENT)pd[i] = kernel_page_directory[i]; else pd[i] = 0;
    }
    return pd;
}

void vmm_switch_address_space(unsigned int *pd) {
    current_page_directory = pd;
    __asm__ volatile("mov %0, %%cr3\n" : : "r"(pd) : "memory");
}

void vmm_destroy_address_space(unsigned int *pd) {
    if (!pd || pd == kernel_page_directory) return;
    for (unsigned int i = 0; i < 1024; i++) {
        if (!(pd[i] & VMM_PRESENT)) continue;
        if ((kernel_page_directory[i] & VMM_PRESENT) && pd[i] == kernel_page_directory[i]) continue;
        unsigned int *pt = (unsigned int *)(pd[i] & 0xFFFFF000);
        for (unsigned int j = 0; j < 1024; j++) {
            if (pt[j] & VMM_PRESENT)
                pmm_free_page((void *)(pt[j] & 0xFFFFF000));
        }
        pmm_free_page(pt);
    }
    pmm_free_page(pd);
}

unsigned int *vmm_get_kernel_page_directory(void) {
    return kernel_page_directory;
}

void vmm_map_page_in(unsigned int *pd, void *virt, void *phys, unsigned int flags) {
    unsigned int v = (unsigned int)virt;
    unsigned int p = (unsigned int)phys;
    unsigned int pd_idx = v >> 22;
    unsigned int pt_idx = (v >> 12) & 0x3FF;
    if (pd != kernel_page_directory
        && pd[pd_idx] & VMM_PRESENT
        && pd[pd_idx] == kernel_page_directory[pd_idx]) {
        unsigned int *new_pt = pmm_alloc_page();
        if (!new_pt) return;
        unsigned int *old_pt = (unsigned int *)(pd[pd_idx] & 0xFFFFF000);
        for (unsigned int j = 0; j < 1024; j++) new_pt[j] = old_pt[j];
        pd[pd_idx] = ((unsigned int)new_pt) | (pd[pd_idx] & 0xFFF);
        if (flags & VMM_USER) pd[pd_idx] |= VMM_USER;
    }

    if (!(pd[pd_idx] & VMM_PRESENT)) {
        unsigned int *pt = pmm_alloc_page();
        if (!pt) return;
        for (unsigned int j = 0; j < 1024; j++) pt[j] = 0;
        pd[pd_idx] = ((unsigned int)pt) | VMM_PRESENT | VMM_WRITABLE | (flags & VMM_USER);
    }
    if (flags & VMM_USER) pd[pd_idx] |= VMM_USER;
    unsigned int *pt = (unsigned int *)(pd[pd_idx] & 0xFFFFF000);
    pt[pt_idx] = (p & 0xFFFFF000) | flags | VMM_PRESENT;
}

void *vmm_alloc_page_in(unsigned int *pd, void *virt, unsigned int flags) {
    void *phys = pmm_alloc_page();
    if (!phys) return 0;
    vmm_map_page_in(pd, virt, phys, flags);
    return phys;
}

unsigned int *vmm_clone_address_space(unsigned int *pd) {
    unsigned int *new_pd = vmm_create_address_space();
    if (!new_pd) return 0;
    for (unsigned int i = 0; i < 1024; i++) {
        if (!(pd[i] & VMM_PRESENT)) continue;
        if ((kernel_page_directory[i] & VMM_PRESENT) && pd[i] == kernel_page_directory[i]) continue;
        unsigned int *old_pt = (unsigned int *)(pd[i] & 0xFFFFF000);
        unsigned int *new_pt = pmm_alloc_page();
        if (!new_pt) { vmm_destroy_address_space(new_pd); return 0; }
        unsigned int pt_failed = 0;
        for (unsigned int j = 0; j < 1024 && !pt_failed; j++) {
            if (!(old_pt[j] & VMM_PRESENT)) { new_pt[j] = 0; continue; }
            void *new_page = pmm_alloc_page();
            if (!new_page) { pt_failed = 1; break; }
            void *old_page = (void *)(old_pt[j] & 0xFFFFF000);
            for (unsigned int k = 0; k < 1024; k++) ((unsigned int *)new_page)[k] = ((unsigned int *)old_page)[k];
            new_pt[j] = (unsigned int)new_page | (old_pt[j] & 0xFFF);
        }

        if (pt_failed) {
            for (unsigned int j = 0; j < 1024; j++) {
                if (new_pt[j] & VMM_PRESENT)
                    pmm_free_page((void *)(new_pt[j] & 0xFFFFF000));
            }
            pmm_free_page(new_pt);
            vmm_destroy_address_space(new_pd);
            return 0;
        }

        new_pd[i] = ((unsigned int)new_pt) | (pd[i] & 0xFFF);
    }

    return new_pd;
}
