/*
 * kernel_stub.c: Routines for basic memory support before paging is enabled
 * Ben Melikant, 25 Apr 2018
 */

#include <stdint.h>
#include <stddef.h>

#ifndef _cplusplus
#include <stdbool.h>
#endif

#include <kmemory.h>
#include <multiboot.h>

#define PAGE_SIZE 4096

// external symbols to assist in locating the memory map
extern int _KHEAP_START;
extern int _KERNEL_TOTAL_SIZE;

// pointer to the memory allocation bitmap (translated to a virtual address later)
physaddr_p memory_bmp;
unsigned long total_blocks;
unsigned long used_blocks;

// function declarations
int kernel_early_init();
int balloc_setup(unsigned long memory_sz);
unsigned long get_memory_size(multiboot_info *mboot);

// kernel_early_init() - do the kernel early initialization (sets up basic paging)
// inputs: multiboot - mulitboot structure pointer
// returns: -1 on error, 0 on success
int kernel_early_init(void *multiboot, unsigned int magic) {

	if (multiboot_data_init(multiboot, magic) == -1) {
		//kstub_panic();
	}
	return 0;
}

// balloc_setup() - perform early initialization of the block allocator
// inputs: memory_sz: size of system memory in bytes(?)
// returns: -1 on error, 0 on success
int balloc_setup(unsigned long memory_sz) {

	// compute the size of the memory map
	total_blocks = (memory_sz / PAGE_SIZE) / sizeof(uint32_t);

	return 0;
}