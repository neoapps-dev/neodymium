#include "heap.h"
#include "vmm.h"
#include "pmm.h"
#define HEAP_START ((void *)0xD0000000)
#define HEAP_INIT_PAGES 16
#define HEAP_ALIGN 8
#define HEAP_MIN_BLOCK 16
struct heap_block {
    unsigned int size;
    struct heap_block *next;
};

static struct heap_block *heap_free;
static unsigned int heap_top;
void heap_init(void) {
    heap_free = 0;
    heap_top = (unsigned int)HEAP_START;
    for (unsigned int i = 0; i < HEAP_INIT_PAGES; i++) {
        if (!vmm_alloc_page((void *)heap_top, VMM_WRITABLE))
            return;
        heap_top += PAGE_SIZE;
    }

    struct heap_block *block = (struct heap_block *)HEAP_START;
    block->size = (HEAP_INIT_PAGES * PAGE_SIZE) | 1;
    block->next = 0;
    heap_free = block;
}

void *malloc(unsigned int size) {
    if (size == 0) return 0;
    unsigned int needed = (sizeof(struct heap_block) + size + HEAP_ALIGN - 1) & ~(HEAP_ALIGN - 1);
    if (needed < HEAP_MIN_BLOCK) needed = HEAP_MIN_BLOCK;
    struct heap_block **pp = &heap_free;
    while (*pp) {
        struct heap_block *block = *pp;
        unsigned int block_size = block->size & ~1u;
        if (block_size >= needed) {
            unsigned int remaining = block_size - needed;
            if (remaining >= HEAP_MIN_BLOCK) {
                block->size = needed & ~1u;
                struct heap_block *split = (struct heap_block *)((char *)block + needed);
                split->size = remaining | 1;
                split->next = block->next;
                *pp = split;
            } else {
                *pp = block->next;
            }
            return (void *)(block + 1);
        }

        pp = &(*pp)->next;
    }

    unsigned int grow = (needed + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
    unsigned int addr = heap_top;
    for (unsigned int i = 0; i < grow / PAGE_SIZE; i++) {
        if (!vmm_alloc_page((void *)(addr + i * PAGE_SIZE), VMM_WRITABLE))
            return 0;
    }
    heap_top = addr + grow;
    struct heap_block *grow_block = (struct heap_block *)addr;
    grow_block->size = grow | 1;
    pp = &heap_free;
    while (*pp && (unsigned int)*pp < addr) pp = &(*pp)->next;
    grow_block->next = *pp;
    *pp = grow_block;
    unsigned int remaining = grow - needed;
    if (remaining >= HEAP_MIN_BLOCK) {
        grow_block->size = needed & ~1u;
        struct heap_block *split = (struct heap_block *)((char *)grow_block + needed);
        split->size = remaining | 1;
        split->next = grow_block->next;
        *pp = split;
    } else {
        *pp = grow_block->next;
    }
    return (void *)(grow_block + 1);
}

void free(void *ptr) {
    if (!ptr) return;
    struct heap_block *block = (struct heap_block *)ptr - 1;
    unsigned int block_size = block->size & ~1u;
    block->size = block_size | 1;
    struct heap_block *prev = 0;
    struct heap_block *curr = heap_free;
    while (curr && (unsigned int)curr < (unsigned int)block) {
        prev = curr;
        curr = curr->next;
    }

    if (curr && (char *)block + block_size == (char *)curr) {
        block->size = (block_size + (curr->size & ~1u)) | 1;
        block->next = curr->next;
    } else {
        block->next = curr;
    }

    if (prev) prev->next = block; else heap_free = block;
    if (prev && (char *)prev + (prev->size & ~1u) == (char *)block) {
        prev->size = ((prev->size & ~1u) + (block->size & ~1u)) | 1;
        prev->next = block->next;
    }
}
