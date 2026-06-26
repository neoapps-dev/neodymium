#ifndef VMM_H
#define VMM_H
#define VMM_PRESENT 0x1
#define VMM_WRITABLE 0x2
#define VMM_USER 0x4
void vmm_init(void);
void vmm_map_page(void *virt, void *phys, unsigned int flags);
void vmm_unmap_page(void *virt);
void *vmm_alloc_page(void *virt, unsigned int flags);
void *vmm_get_phys(void *virt);
#endif
