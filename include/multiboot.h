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

/** Multiboot function error return codes */

#define MBOOT_NOT_IMPLEMENTED			0xffff

#define MBOOT_ERROR_INVALID_MEMSIZE		0x0001
#define MBOOT_ERROR_INVALID_MEMORY_MAP	0x0002
#define MBOOT_ERROR_NONCOMPLIANT_LOADER	0x0003
#define MBOOT_ERROR_MISALIGNED_HEADER	0x0004
#define MBOOT_ERROR_INVALID_BOOTSTRUCT	0x0005
#define MBOOT_ERROR_ENTRY_NOT_FOUND		0x0006
#define MBOOT_ERROR_UNSUPPORTED			0x0007

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

// multiboot_info structure

struct MULTIBOOT_INFO {

	uint32_t flags;					// this field is required. Structure flags
	uint32_t memorySzLo;			// memory size (if flags[0] is set)
	uint32_t memorySzHi;
	uint32_t bootDevice;			// boot device (if flags[1] is set)
	uint32_t commandLine;			// kernel command line (if flags[2] is set)
	uint32_t mods_count;			// # of modules loaded (if flags[3] is set)
	uint32_t mods_addr;				// address of modules (if flags[3] is set)
	uint32_t elf_symbol_count;		// how many section symbols are present?
	uint32_t elf_entry_sz;			// how big is each symbol table entry?
	uint32_t elf_table_addr;		// where is the ELF symbol table located?
	uint32_t elf_string_table;		// strings representing ELF symbols
	uint32_t mmap_length;			// length of memory map (if flags[6] is set)
	uint32_t mmap_addr;				// address of memory map
	uint32_t drives_length;			// address & length of first drive structure
	uint32_t drives_addr;
	uint32_t config_table;			// ROM configuration table
	uint32_t bootldr_name;			// name of bootloader
	uint32_t apm_table;				// Advanced power management table
	uint32_t vbe_ctrl_info;			// information for video mode
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;

}__attribute__((packed));

// GRUB memory map structure info

struct MEMORY_MAP_INF {

        uint32_t sz;            // size of this structure
        uint64_t baseAddr;      // base address of the memory region
        uint64_t length;        // memory section length
        uint32_t type;          // type of the region
 
}__attribute__((packed));

// GRUB boot modules structure

struct MBOOT_MODULE_INF {

	unsigned int 	mod_addr_start;
	unsigned int 	mod_addr_end;
	const char 	   *mod_string;

}__attribute__((packed));

/* Multiboot2 standard information */

#define MULTIBOOT2_MAGIC 0x36d76289

#define MBOOT2_TYPETAG_END		0x00
#define MBOOT2_TYPETAG_MEMSZ	0x04
#define MBOOT2_TYPETAG_MMAP		0x06
#define MBOOT2_TYPETAG_BLNAME	0x02
#define MBOOT2_TYPETAG_CMDLINE	0x01
#define MBOOT2_TYPETAG_MODULES	0x03
#define MBOOT2_TYPETAG_ELFSYM	0x09

/* base multiboot tag. every tag starts with this entry */
struct MULTIBOOT2_TAG {

	uint16_t type;
	uint16_t flags;
	uint32_t size;

}__attribute__((packed));

/* memory size tag. contains info about how much memory is available */
struct MULTIBOOT2_TAG_MEMSZ {

	struct MULTIBOOT2_TAG tag;
	uint32_t mem_lower;
	uint32_t mem_upper;

}__attribute__((packed));

/* memory map entry structure for multiboot2 */
struct MULTIBOOT2_ENTRY_MMAP {

	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;

}__attribute__((packed));

/* memory map tag. contains info on memory layout for the system. */
struct MULTIBOOT2_TAG_MMAP {

	struct MULTIBOOT2_TAG tag;
	uint32_t entry_sz;
	uint32_t entry_ver;
	struct MULTIBOOT2_ENTRY_MMAP entries[0];

}__attribute__((packed));

typedef struct MEMORY_MAP_DATA {

	uint64_t base;
	uint64_t length;
	uint32_t type;

} mmap_data;

/* multiboot / multiboot2 typedefs */
typedef struct MULTIBOOT_INFO multiboot_info;
typedef struct MEMORY_MAP_INF memory_map_inf;
typedef struct MBOOT_MODULE_INF mboot_module_inf;

typedef struct MULTIBOOT2_TAG 			multiboot2_tag;
typedef struct MULTIBOOT2_TAG_MEMSZ 	multiboot2_tag_memsz;
typedef struct MULTIBOOT2_ENTRY_MMAP 	multiboot2_entry_mmap;
typedef struct MULTIBOOT2_TAG_MMAP 		multiboot2_tag_mmap;

/* Multiboot parsing functions */

int 			multiboot_data_init 		(void *mb_inf, unsigned int boot_magic);
unsigned int 	multiboot_get_memsz 		();
mmap_data      *multiboot_get_mmap_next 	(mmap_data *data);
void		   *multiboot_find_rsdp			();

#ifdef _cplusplus
}
#endif

#endif
