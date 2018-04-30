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
#include <early_terminal.h>

#define PAGE_SIZE 4096

// external symbols to assist in locating the memory map
extern uint32_t _KHEAP_START;
extern int _KERNEL_TOTAL_SIZE;

extern void early_panic();

// pointer to the memory allocation bitmap (translated to a virtual address later)
physaddr_p memory_bmp;
unsigned long total_blocks;
unsigned long used_blocks;

// function declarations
int kernel_early_init();
int balloc_early_setup(unsigned long memory_sz);

unsigned long get_memory_size(multiboot_info *mboot);

// kernel_early_init() - do the kernel early initialization (sets up basic paging)
// inputs: multiboot - mulitboot structure pointer
// returns: -1 on error, 0 on success
int kernel_early_init(void *multiboot, unsigned int magic) {

	kernel_early_printf("%L Kernel is coming up\n");
	kernel_early_printf("%L Getting multiboot information\n");

	if (multiboot_data_init(multiboot, magic) == -1) {
		kernel_early_printf("%P Error fetching multiboot data\n");
		early_panic();
	}

	kernel_early_printf("%L Memory size reported by multiboot: %d kilobytes\n", multiboot_get_memsz());
	
	if (balloc_early_setup(multiboot_get_memsz()) == -1) {
		kernel_early_printf("%P Error initializing block allocator\n");
		early_panic();
	}

	return 0;
}

// balloc_setup() - perform early initialization of the block allocator
// inputs: memory_sz: size of system memory in bytes(?)
// returns: -1 on error, 0 on success
int balloc_early_setup(unsigned long memory_sz) {

	// compute the size of the memory map
	total_blocks = ((memory_sz * 1024) / PAGE_SIZE) / sizeof(uint32_t);
	kernel_early_printf("%I Memory allocator total blocks: %d\n", total_blocks);
	memory_bmp = (physaddr_p) &_KHEAP_START;

	// zero out the bitmap
	

	return 0;
}

