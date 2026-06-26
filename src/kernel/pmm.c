#include "pmm.h"
#include "../boot/multiboot.h"
#define PMM_BITMAP_SIZE (PMM_MAX_PAGES / 32)
static unsigned int pmm_bitmap[PMM_BITMAP_SIZE];
static unsigned int free_count;
static unsigned int total_pages;
extern unsigned int _kernel_start;
extern unsigned int _kernel_end;
void pmm_init(struct multiboot_info *mbd) {
    unsigned int mem_size = 0;
    if (mbd->flags & (1 << 0))
        mem_size = mbd->mem_upper + 1024;
    unsigned int total_mem = mem_size * 1024;
    total_pages = total_mem / PAGE_SIZE;
    if (total_pages > PMM_MAX_PAGES)
        total_pages = PMM_MAX_PAGES;
    free_count = total_pages;
    pmm_bitmap[0] |= 1;
    free_count--;
    unsigned int ksp = (unsigned int)&_kernel_start / PAGE_SIZE;
    unsigned int kep = ((unsigned int)&_kernel_end + PAGE_SIZE - 1) / PAGE_SIZE;
    for (unsigned int i = ksp; i < kep; i++) {
        if (!((pmm_bitmap[i / 32] >> (i % 32)) & 1)) {
            pmm_bitmap[i / 32] |= (1 << (i % 32));
            free_count--;
        }
    }

    if (mbd->flags & (1 << 6)) {
        struct multiboot_mmap_entry *mmap = (struct multiboot_mmap_entry *)mbd->mmap_addr;
        unsigned int mmap_end = mbd->mmap_addr + mbd->mmap_length;
        while ((unsigned int)mmap < mmap_end) {
            if (mmap->type != 1 && mmap->addr_low < total_mem) {
                unsigned int start = mmap->addr_low;
                unsigned int end = mmap->addr_low + mmap->len_low;
                if (end > total_mem) end = total_mem;
                unsigned int sp = (start + PAGE_SIZE - 1) / PAGE_SIZE;
                unsigned int ep = (end + PAGE_SIZE - 1) / PAGE_SIZE;
                for (unsigned int i = sp; i < ep && i < total_pages; i++) {
                    if (!((pmm_bitmap[i / 32] >> (i % 32)) & 1)) {
                        pmm_bitmap[i / 32] |= (1 << (i % 32));
                        free_count--;
                    }
                }
            }
            mmap = (struct multiboot_mmap_entry *)((unsigned int)mmap + mmap->size + 4);
        }
    }
}

void *pmm_alloc_page(void) {
    if (free_count == 0)
        return 0;
    for (unsigned int i = 0; i < total_pages; i++) {
        if (!((pmm_bitmap[i / 32] >> (i % 32)) & 1)) {
            pmm_bitmap[i / 32] |= (1 << (i % 32));
            free_count--;
            return (void *)(i * PAGE_SIZE);
        }
    }
    return 0;
}

void pmm_free_page(void *addr) {
    unsigned int page = (unsigned int)addr / PAGE_SIZE;
    if (page < total_pages && ((pmm_bitmap[page / 32] >> (page % 32)) & 1)) {
        pmm_bitmap[page / 32] &= ~(1 << (page % 32));
        free_count++;
    }
}

void *pmm_alloc_pages(unsigned int count) {
    if (count == 0 || free_count < count)
        return 0;
    for (unsigned int i = 0; i <= total_pages - count; i++) {
        int free = 1;
        for (unsigned int j = 0; j < count; j++) {
            if ((pmm_bitmap[(i + j) / 32] >> ((i + j) % 32)) & 1) {
                free = 0;
                break;
            }
        }
        if (free) {
            for (unsigned int j = 0; j < count; j++) {
                pmm_bitmap[(i + j) / 32] |= (1 << ((i + j) % 32));
                free_count--;
            }
            return (void *)(i * PAGE_SIZE);
        }
    }
    return 0;
}

void pmm_free_pages(void *addr, unsigned int count) {
    unsigned int start_page = (unsigned int)addr / PAGE_SIZE;
    for (unsigned int i = 0; i < count; i++) {
        unsigned int page = start_page + i;
        if (page < total_pages && ((pmm_bitmap[page / 32] >> (page % 32)) & 1)) {
            pmm_bitmap[page / 32] &= ~(1 << (page % 32));
            free_count++;
        }
    }
}

unsigned int pmm_get_free_page_count(void) {
    return free_count;
}
