#include "efi.h"
#include "../kernel/printf.h"
#define EFI_MMAP_MAX_ENTRIES 256
static struct multiboot_mmap_entry efi_mmap_buf[EFI_MMAP_MAX_ENTRIES];
static int efi_type_to_mboot(uint32_t efi_type) {
    switch (efi_type) {
        case EfiConventionalMemory:
        case EfiLoaderCode:
        case EfiLoaderData:
            return 1;
        case EfiBootServicesCode:
        case EfiBootServicesData:
            return 1;
        case EfiACPIReclaimMemory:
            return 3;
        case EfiACPIMemoryNVS:
            return 4;
        default:
            return 2;
    }
}
void efi_populate_multiboot(void *mmap_ptr, uint64_t mmap_size,
                            uint64_t desc_size, uint32_t desc_ver,
                            efi_graphics_output_protocol *gop,
                            struct multiboot_info *out) {
    (void)desc_ver;
    unsigned int count = 0;
    uint64_t total_mem = 0;
    uint8_t *ptr = (uint8_t *)mmap_ptr;
    for (uint64_t off = 0; off < mmap_size && count < EFI_MMAP_MAX_ENTRIES;
         off += desc_size) {
        efi_memory_descriptor *desc = (efi_memory_descriptor *)(ptr + off);
        if (desc->type == EfiConventionalMemory)
            total_mem += desc->number_of_pages * 4096;
        struct multiboot_mmap_entry *e = &efi_mmap_buf[count];
        e->addr_low = (uint32_t)(desc->physical_start & 0xFFFFFFFF);
        e->addr_high = (uint32_t)(desc->physical_start >> 32);
        e->len_low = (uint32_t)((desc->number_of_pages * 4096) & 0xFFFFFFFF);
        e->len_high = (uint32_t)((desc->number_of_pages * 4096) >> 32);
        e->type = efi_type_to_mboot(desc->type);
        e->reserved = 0;
        count++;
    }
    out->flags = (1 << 0) | (1 << 6);
    if (total_mem > 1024 * 1024)
        out->mem_upper = (unsigned int)((total_mem - 1024 * 1024) / 1024);
    else
        out->mem_upper = 0;
    out->mmap_addr = (uint32_t)efi_mmap_buf;
    out->mmap_length = count * sizeof(struct multiboot_mmap_entry);
    out->mmap_entry_size = sizeof(struct multiboot_mmap_entry);
    if (gop && gop->mode) {
        efi_graphics_output_mode *mode = gop->mode;
        if (mode->info) {
            out->flags |= (1 << 12);
            out->framebuffer_addr = mode->frame_buffer_base;
            out->framebuffer_pitch = mode->info->pixels_per_scan_line * 4;
            out->framebuffer_width = mode->info->horizontal_resolution;
            out->framebuffer_height = mode->info->vertical_resolution;
            out->framebuffer_bpp = 32;
            out->framebuffer_type = 2;
            printf("[efi] gop %ux%u fb=0x%llx\n",
                   mode->info->horizontal_resolution,
                   mode->info->vertical_resolution,
                   mode->frame_buffer_base);
        }
    }
    printf("[efi] boot services memory map: %u entries, %u KiB conventional\n",
           count, (unsigned int)(total_mem / 1024));
}
