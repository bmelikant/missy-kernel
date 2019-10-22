/**
 * This file implements sbrk functionality for the kernel address space
 */

#include <kernel/memory.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#include "pagemngr.h"
#include "balloc.h"

void *_brk = NULL;
void *_heap_base = NULL;

void memory_setbase(void *base_address) {
	_heap_base = base_address;
}

void memory_init_mmap(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks) {
	balloc_initialize(mbmp, total_blocks, used_blocks);
}

/**
 * Set the new program break (end of heap space for this program)
 * This should be called at the start of each process to set the
 * base data segment
 */
int brk(void *addr) {
	puts("starting brk()");
	if (!_heap_base) {
		puts("_heap_base is not intitialized");
		errno = EINVAL;
		return -1;
	}

	// make sure we aren't trying to set the break to some
	// undefined region of memory before the heap start
	uintptr_t __heap_base_addr = (uintptr_t)(_heap_base);
	uintptr_t __new_brk_addr = (uintptr_t)(addr);

	printf("_heap_base: 0x%x\n", (unsigned int)(_heap_base));
	printf("_addr: 0x%x\n", (unsigned int)(addr));

	if (__new_brk_addr < __heap_base_addr) {
		errno = ENOMEM;
		return -1;
	}

	// page-align the new brk addr to the next page up
	uintptr_t __page_aligned_top = PAGEMNGR_PAGE_ALIGN(__new_brk_addr);

	// fresh allocation 
	if (!_brk) {
		int block_count = (__page_aligned_top-__heap_base_addr)/PAGEMNGR_PAGE_SIZE;
		void *frame_addr = __heap_base_addr;

		// allocate and map blocks until block_count is satisifed
		for (int i = 0; i < block_count; i++) {
			void *block = balloc_allocate_block();
			if (!block) {
				while (i >= 0) {
					void *physical = pagemngr_unmap_block(frame_addr);
					balloc_deallocate_block(physical);
					frame_addr -= PAGEMNGR_PAGE_SIZE;
				}
				errno = ENOMEM;
				return -1;
			}

			pagemngr_map_block(block,frame_addr);
			frame_addr += PAGEMNGR_PAGE_SIZE;
		}
	} else {
	}
}