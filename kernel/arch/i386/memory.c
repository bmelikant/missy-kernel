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

void memory_init_mmap(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks) {
	balloc_initialize(mbmp, total_blocks, used_blocks);
}

int memory_get_blocks(void *requested_addr, size_t block_count) {
	void *map_address = (void *) ALIGN_BLOCK_DOWN((uintptr_t) requested_addr);
	for (size_t i = 0; i < block_count; i++) {
		void *block = balloc_allocate_block();
		if (!block) {
			while (i > 0) {
				void *physical = pagemngr_unmap_block(map_address);
				if (physical) {
					balloc_deallocate_block(physical);
					map_address -= PAGE_SIZE;
				}
				errno = ENOMEM;
				return -1;
			}
		}
		pagemngr_map_block(block,map_address);
		map_address += PAGE_SIZE;
	}
	return 0;
}