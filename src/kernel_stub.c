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
#include <early_utils.h>
#include <early_errno.h>

#define PAGE_SIZE 4096
#define MEMORY_BLOCK_FULL 0xffffffff
#define BYTE_TOTAL_SIZE 8

// macro definitions
#define early_memory_bitmap_clear(bit)	(memory_bmp[bit/32] = (memory_bmp[bit/32] &~(1 << (bit%32))))
#define early_memory_bitmap_set(bit)	(memory_bmp[bit/32] = (memory_bmp[bit/32] | (1 << (bit%32))))
#define early_memory_bitmap_test(bit)	(memory_bmp[bit/32] & (1<<(bit%32)))

// external symbols to assist in locating the memory map
extern uint32_t _KHEAP_START;
extern int _KERNEL_TOTAL_SIZE;

extern void early_panic();

// pointer to the memory allocation bitmap (translated to a virtual address later)
physaddr_p memory_bmp;
long total_blocks;
long used_blocks;

// function declarations
int kernel_early_init();
int balloc_early_setup();
void *balloc_early_get();

static size_t balloc_early_first_free();
static void balloc_reserve_region(long start, size_t size);
static void balloc_free_region(long start, size_t size);

unsigned long get_memory_size(multiboot_info *mboot);

// kernel_early_init() - do the kernel early initialization (sets up basic paging)
// inputs: multiboot - mulitboot structure pointer
// returns: -1 on error, 0 on success
int kernel_early_init(void *multiboot, unsigned int magic) {

	kernel_early_printf("%L Kernel is coming up\n");
	kernel_early_printf("%L Getting multiboot information\n");
	kernel_early_printf("%I Multiboot magic is reported as %i\n", magic);

	int multiboot_return = multiboot_data_init(multiboot,magic);

	if (multiboot_data_init(multiboot, magic) == -1) {
		kernel_early_printf("%P Error fetching multiboot data\n");
		switch(early_errno) {
			case MBOOT_ERROR_INVALID_MEMSIZE:
				kernel_early_printf("%P Could not find a memory size entry in the multiboot structure!\n");
				break;
			case MBOOT_ERROR_INVALID_MEMORY_MAP:
				kernel_early_printf("%P Could not find the memory map from the multiboot structure!\n");
				break;
			case MBOOT_ERROR_NONCOMPLIANT_LOADER:
				kernel_early_printf("%P Multiboot loader is not compliant with either multiboot or multiboot2 specification!\n");
				break;
			default:
				kernel_early_printf("%P An unknown error has occured. Error code: %i\n", early_errno);
				break;
		}
		early_panic();
	}

	kernel_early_printf("%L Memory size reported by multiboot: %d kilobytes\n", multiboot_get_memsz());
	
	if (balloc_early_setup() == -1) {
		kernel_early_printf("%P Error initializing block allocator\n");
		early_panic();
	}

	kernel_early_printf("%I Block allocator has been initialized");

	return 0;
}

// balloc_setup() - perform early initialization of the block allocator
// inputs: none
// returns: -1 on error, 0 on success
int balloc_early_setup() {

	// compute the size of the memory map
	unsigned long memory_sz = multiboot_get_memsz();
	total_blocks = ((memory_sz * 1024) / PAGE_SIZE) / sizeof(uint32_t);
	kernel_early_printf("%I Memory allocator total blocks: %d\n", total_blocks);
	memory_bmp = (physaddr_p) &_KHEAP_START;

	// mark everything as reserved
	early_memset(memory_bmp,0xff,total_blocks*sizeof(uint32_t));

	// use the memory map to clear free regions
	mmap_data map_entry;
	early_memset(&map_entry,0,sizeof(mmap_data));
	
	while (multiboot_get_mmap_next(&map_entry)) {

		uint32_t base = map_entry.base >> 32;
		base &= 0xffffffff;
		uint32_t length = map_entry.length >> 32;
		base &= 0xffffffff;

		kernel_early_printf("%L Memory region - start: 0x%x, length: 0x%x, type: %d\n", base, length, map_entry.type);
		// unused memory; free it
		if (map_entry.type == 1) {
		}

		early_memset(&map_entry,0,sizeof(mmap_data));
	}

	return 0;
}

// balloc_get(): Return a free block of memory from the block allocator
// inputs: none
// returns: free block if block available, NULL otherwise
void *balloc_early_get() {
	size_t free_block = balloc_early_first_free();
	
	if (!free_block) {
		return NULL;
	}

	early_memory_bitmap_set(free_block);
	return (void *) free_block;
}

static size_t balloc_early_first_free() {
	// find the first free block
	for (int i = 0; i < (int) total_blocks; i++) {
		if (memory_bmp[i] != MEMORY_BLOCK_FULL) {
			// find the first free block in this segment
			for (int j = 0; j < 32; j++) {
				if (32 & (1 << j)) {
					return i*32+j;
				}
			}
		}
	}

	return 0;
}

// paging_early_setup() - configure basic paging
// inputs:
// returns: -1 on error, 0 on success (PAGING IS ENABLED AFTER THIS FUNCTION CALL!!)
int paging_early_setup() {

	// grab a block from the allocator and identity map the first megabyte, and the early initializer

	return 0;
}