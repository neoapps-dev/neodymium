#ifndef PMM_H
#define PMM_H
#define PAGE_SIZE 4096
#define PMM_MAX_PAGES 1048576
struct multiboot_info;
void pmm_init(struct multiboot_info *mbd);
void *pmm_alloc_page(void);
void pmm_free_page(void *addr);
void *pmm_alloc_pages(unsigned int count);
void pmm_free_pages(void *addr, unsigned int count);
unsigned int pmm_get_free_page_count(void);
#endif
