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
void vmm_free_page(void *virt);
unsigned int *vmm_get_kernel_page_directory(void);
unsigned int *vmm_create_address_space(void);
void vmm_switch_address_space(unsigned int *pd);
void vmm_destroy_address_space(unsigned int *pd);
void vmm_map_page_in(unsigned int *pd, void *virt, void *phys, unsigned int flags);
void *vmm_alloc_page_in(unsigned int *pd, void *virt, unsigned int flags);
unsigned int *vmm_clone_address_space(unsigned int *pd);
#endif
