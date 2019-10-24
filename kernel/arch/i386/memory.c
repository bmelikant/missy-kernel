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

static int allocate_new_brk(void *frame_addr, size_t new_block_count);

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

	if (__new_brk_addr < __heap_base_addr) {
		errno = ENOMEM;
		return -1;
	}

	// page-align the new brk addr to the next page up
	uintptr_t __page_aligned_top = PAGEMNGR_PAGE_ALIGN_UP(__new_brk_addr);

	// fresh allocation 
	if (!_brk) {
		size_t block_count = (__page_aligned_top-__heap_base_addr)/PAGEMNGR_PAGE_SIZE;
		void *frame_addr = (void *) __heap_base_addr;
		printf("blocks to allocate: %u\n", block_count);
		printf("allocate to 0x%x\n", (uint32_t) frame_addr);

		// allocate and map blocks until block_count is satisifed
		if (allocate_new_brk(frame_addr, block_count) == -1) return -1;
		_brk = addr;

	} else {
		uintptr_t __old_brk_addr = (uintptr_t) _brk;

		// new break is inside the last mapped page
		if (PAGEMNGR_PAGE_ALIGN_DOWN(__new_brk_addr) == PAGEMNGR_PAGE_ALIGN_DOWN(__old_brk_addr)) {
			_brk = addr;
		} else {
			size_t new_block_count = __page_aligned_top-PAGEMNGR_PAGE_ALIGN_DOWN(__old_brk_addr);
			void *frame_addr = (void *) PAGEMNGR_PAGE_ALIGN_UP(__old_brk_addr);
			printf("new blocks to allocate: %u\n", new_block_count);
			printf("allocate to 0x%x\n", (uint32_t) frame_addr);

			if (allocate_new_brk(frame_addr,new_block_count) == -1) return -1;
			_brk = addr;
		}
	}
	return 0;
}

static int allocate_new_brk(void *frame_addr, size_t new_block_count) {
	for (size_t i = 0; i < new_block_count; i++) {
		void *block = balloc_allocate_block();
		if (!block) {
			while (i > 0) {
				void *physical = pagemngr_unmap_block(frame_addr);
				if (physical) {
					balloc_deallocate_block(physical);
					frame_addr -= PAGEMNGR_PAGE_SIZE;
				}
				errno = ENOMEM;
				return -1;
			}
		}
		pagemngr_map_block(block,frame_addr);
		frame_addr += PAGEMNGR_PAGE_SIZE;
	}
	return 0;
}