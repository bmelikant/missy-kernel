/*
 * multiboot.c: Functions for parsing information in the
 * multiboot and multiboot2 standards
 *
 *  Created on: Aug 21, 2017
 *      Author: bmelikant
 */

#include <init/multiboot/multiboot.h>
#include <init/kerrno.h>
#include <init/kterm.h>
#include <init/kutils.h>

// internal data
unsigned int boot_magic = 0;
unsigned int mb_memory_sz = 0;

void *mboot_ptr = NULL;
void *mmap_ptr = NULL;
static multiboot_api_t api_struct;

multiboot_fn multiboot_fns[FN_COUNT];

/** initialization functions */
extern int init_multiboot(multiboot_api_t *api);
extern int init_multiboot2(multiboot_api_t *api);

static void init_api_struct();

/**
 * Initialize the multiboot driver. The correct driver is selected based on the multiboot magic number
 * If this function does not return 0, it is NOT safe to call any multiboot functions
 * inputs: 
 * void *multiboot_ptr: the address of the multiboot struct in memory
 * unsigned int header_magic: the multiboot magic number reported by the loader
 * returns:
 * 	0 on success, -1 on failure (kinit_errno contains extended error code)
 */
int multiboot_init(void *multiboot_ptr, unsigned int header_magic) {
	// the multiboot header must be word-aligned
	if ((unsigned long) multiboot_ptr & 7) {
		kinit_errno = MBOOT_ERROR_MISALIGNED_HEADER;
		return -1;
	}
	
	// set up the local multiboot fields
	boot_magic = header_magic;
	mboot_ptr = multiboot_ptr;

	init_api_struct();

	if (boot_magic == MULTIBOOT_MAGIC) {
		#ifdef DEBUG_MULTIBOOT
		kernel_early_printf("Multiboot1-compliant loader\n");
		#endif
		return init_multiboot(&api_struct);
	} else if (boot_magic == MULTIBOOT2_MAGIC) {
		#ifdef DEBUG_MULTIBOOT
		kernel_early_printf("Multiboot2-compliant loader\n");
		#endif
		return init_multiboot2(&api_struct);
	} else {
		#ifdef DEBUG_MULTIBOOT
		kernel_early_printf("Non-compliant loader\n");
		#endif
		kinit_errno = MBOOT_ERROR_UNSUPPORTED;
		return -1;
	}
}

/**
 * unsigned int multiboot_get_memsz (): Get the size of physical memory from multiboot
 * inputs: none
 * returns: size of memory in KB
 */
unsigned int multiboot_get_memsz () {
	unsigned int memsize = 0;
	if (api_struct.get_memory_size(&memsize) != -1) {
		#ifdef DEBUG_MULTIBOOT
		kernel_early_printf("Multiboot API found memory size: %d\n", memsize);
		#endif
	} else {
		#ifdef DEBUG_MULTIBOOT
		kernel_early_printf("Error getting memory size: 0x%x\n", kinit_errno);
		#endif
	}
	return memsize;
}

/**
 * int multiboot_get_mmap_next (): Get the next entry from the memory map
 * inputs: buf - the buffer for the entry data
 * returns: * to buf (NULL if at end of list)
 */
int multiboot_get_mmap_next(mmap_data *buf) {
	if (api_struct.get_next_mmap_entry((void *) buf) != -1) {
		#ifdef DEBUG_MULTIBOOT
		kernel_early_puts("Found next multiboot entry\n");
		#endif
		return 0;
	}
	#ifdef DEBUG_MULTIBOOT
	kernel_early_puts("current memory map entry is invalid. done reading?\n");
	#endif
	return -1;
}

/**
 * void multiboot_relocate(): relocate the multiboot structure to a new position above the kernel
 * This is done to avoid overwriting the multiboot structure with any physical memory allocation data
 * This issue is only known to affect grub2 bootloader at this point, original grub just skips this code
 */
void multiboot_relocate(void *kernel_end_ptr) {
	api_struct.relocate_multiboot(kernel_end_ptr);
}

int multiboot_unimplemented(__attribute__((unused)) void *ignored) {
	#ifdef DEBUG_MULTIBOOT
	kernel_early_puts("Unimplemented API call");
	#endif
	return 0;
}

void init_api_struct() {
	api_struct.get_memory_size = multiboot_unimplemented;
	api_struct.get_next_mmap_entry = multiboot_unimplemented;
	api_struct.relocate_multiboot = multiboot_unimplemented;
}
