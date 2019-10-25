#ifndef _MISSY_KERNEL_MULTIBOOT_H
#define _MISSY_KERNEL_MULTIBOOT_H

#ifdef _cplusplus
extern "C" {
#endif

// standard includes

#if !defined(_cplusplus)
#include <stdbool.h>
#endif

#include <stddef.h>
#include <stdint.h>

/** Multiboot function errno codes */

#define MBOOT_NOT_IMPLEMENTED			0xffff

#define MBOOT_ERROR_INVALID_MEMSIZE		0x0001
#define MBOOT_ERROR_INVALID_MEMORY_MAP	0x0002
#define MBOOT_ERROR_NONCOMPLIANT_LOADER	0x0003
#define MBOOT_ERROR_MISALIGNED_HEADER	0x0004
#define MBOOT_ERROR_INVALID_BOOTSTRUCT	0x0005
#define MBOOT_ERROR_ENTRY_NOT_FOUND		0x0006
#define MBOOT_ERROR_UNSUPPORTED			0x0007

/** multiboot function error return code */
#define MBOOT_ERROR 0x01

/* Multiboot (original) standard definitions */

// bit-test macros for the boot structure flags
#define bootflag_check(x,y) (x&y)

// bit definitions for boot flags

#define MEMSZ_PRESENT 			0x0001
#define BOOTDEV_PRESENT 		0x0002
#define CMDLINE_PRESENT 		0x0004
#define MODULES_PRESENT 		0x0008
#define AOUT_SYMTABLE_PRESENT	0x0010
#define ELF_SYMTABLE_PRESENT 	0x0020
#define MMAP_PRESENT    		0x0040
#define DRIVES_PRESENT  		0x0080
#define CONFIG_PRESENT      	0x0100
#define BOOTLDR_NAME_PRESENT 	0x0200
#define APM_TABLE_PRESENT		0x0400
#define VBE_TABLE_PRESENT		0x0800

#define MULTIBOOT_MAGIC 0x2badb002
#define MULTIBOOT2_MAGIC 0x36d76289

/** ugly hack: using FN_COUNT to determine the number of functions stored in this enum */
typedef enum MULTIBOOT_FN_IDX {
	MEMORY_SIZE = 0, NEXT_MMAP_ENTRY, MBOOT_HDR_SZ, FN_COUNT
} multiboot_fn_idx;

typedef int (*multiboot_fn)(void *data);

typedef struct MULTIBOOT_API {
	multiboot_fn get_memory_size;
	multiboot_fn get_next_mmap_entry;
	multiboot_fn relocate_multiboot;
	multiboot_fn get_rsdp;
} multiboot_api_t;

typedef struct MEMORY_MAP_DATA {
	uint64_t base;
	uint64_t length;
	uint32_t type;
} mmap_data;

typedef struct ACPI1_RSDP {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_addr;
}__attribute__((packed)) _acpi_rsdp_t;

typedef struct ACPI2_RSDP {
	_acpi_rsdp_t rdst_original;
	uint32_t length;
	uint64_t xsdt_addr;
	uint8_t checksum_extended;
	uint8_t reserved[3];
}__attribute__((packed)) _acpi2_rsdp_t;


/* Multiboot parsing functions */
int				multiboot_init				(void *multiboot_ptr, unsigned int header_magic);
void			multiboot_relocate			(void *_kernel_end);
unsigned int 	multiboot_get_memsz 		();
int				multiboot_get_mmap_next 	(mmap_data *data);
int				multiboot_get_rsdp			(_acpi2_rsdp_t *dest);

#ifdef _cplusplus
}
#endif

#endif
