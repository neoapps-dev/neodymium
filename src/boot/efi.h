#ifndef EFI_H
#define EFI_H
#include "multiboot.h"
#define EFI_SUCCESS 0
#define EFI_NOT_FOUND 14
#define EFI_BUFFER_TOO_SMALL 5
#define EfiReservedMemoryType 0
#define EfiLoaderCode 1
#define EfiLoaderData 2
#define EfiBootServicesCode 3
#define EfiBootServicesData 4
#define EfiRuntimeServicesCode 5
#define EfiRuntimeServicesData 6
#define EfiConventionalMemory 7
#define EfiUnusableMemory 8
#define EfiACPIReclaimMemory 9
#define EfiACPIMemoryNVS 10
#define EfiMemoryMappedIO 11
#define EfiMemoryMappedIOPortSpace 12
#define EfiPalCode 13
typedef unsigned long long efi_uintn_t;
typedef unsigned long long efi_status_t;
typedef void *efi_handle_t;
typedef unsigned short efi_char16_t;
typedef struct {
    uint32_t data1;
    uint16_t data2;
    uint16_t data3;
    uint8_t data4[8];
} __attribute__((packed)) efi_guid_t;
typedef struct {
    uint32_t type;
    uint32_t pad;
    uint64_t physical_start;
    uint64_t virtual_start;
    uint64_t number_of_pages;
    uint64_t attribute;
} __attribute__((packed)) efi_memory_descriptor;
typedef struct {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
    efi_status_t (*raise_tpl)(efi_uintn_t);
    void (*restore_tpl)(efi_uintn_t, efi_uintn_t);
    efi_status_t (*allocate_pages)(int, int, efi_uintn_t, uint64_t *);
    efi_status_t (*free_pages)(uint64_t, efi_uintn_t);
    efi_status_t (*get_memory_map)(efi_uintn_t *, void *, efi_uintn_t *,
                                   efi_uintn_t *, uint32_t *);
    efi_status_t (*allocate_pool)(int, efi_uintn_t, void **);
    efi_status_t (*free_pool)(void *);
} __attribute__((packed)) efi_boot_services;
typedef struct {
    uint64_t signature;
    uint32_t revision;
    uint32_t header_size;
    uint32_t crc32;
    uint32_t reserved;
    efi_char16_t *firmware_vendor;
    uint32_t firmware_revision;
    efi_handle_t console_in_handle;
    void *simple_text_input;
    efi_handle_t console_out_handle;
    void *simple_text_output;
    efi_handle_t standard_error_handle;
    void *simple_stderr;
    efi_boot_services *boot_services;
    uint64_t number_of_table_entries;
    void **configuration_table;
} __attribute__((packed)) efi_system_table;
typedef struct {
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t reserved_mask;
} efi_pixel_bitmask;
typedef struct {
    uint32_t version;
    uint32_t horizontal_resolution;
    uint32_t vertical_resolution;
    efi_pixel_bitmask pixel_format;
    uint32_t pixels_per_scan_line;
} __attribute__((packed)) efi_graphics_output_mode_info;
typedef struct {
    uint32_t max_mode;
    uint32_t mode;
    efi_graphics_output_mode_info *info;
    uint64_t size_of_info;
    uint64_t frame_buffer_base;
    uint64_t frame_buffer_size;
} efi_graphics_output_mode;
typedef struct {
    efi_status_t (*query_mode)(void *, uint32_t, uint64_t *,
                               efi_graphics_output_mode_info **);
    efi_status_t (*set_mode)(void *, uint32_t);
    efi_status_t (*blt)(void *, ...);
    efi_graphics_output_mode *mode;
} efi_graphics_output_protocol;
#endif
