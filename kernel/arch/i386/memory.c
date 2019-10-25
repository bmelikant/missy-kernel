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

#define FIRST_TEMPORARY_PAGE	0xffbf8000			// first page above the stack up to 0xffc0000
#define LAST_TEMPORARY_PAGE		0xffbff000			// last page before the recursive mapping
#define TEMP_BITMAP_SIZE		8

static uint8_t _temp_page_bitmap = 0;

void memory_init_mmap(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks) {
	balloc_initialize(mbmp, total_blocks, used_blocks);
}

void *memory_request_temp_map(void *physical) {
	uintptr_t physical_offset = (uintptr_t)(physical) & 0xfff;
	physical = ALIGN_BLOCK_DOWN((uintptr_t) physical);
	printf("physical_offset: 0x%x\n", (uint32_t) physical_offset);
	printf("physical: 0x%x\n", (uint32_t) physical);

	// find a free entry
	for (size_t i = 0; i < TEMP_BITMAP_SIZE; i++) {
		if ((_temp_page_bitmap & (1 << i)) == 0) {
			void *temp_virtual = (void *) FIRST_TEMPORARY_PAGE + (i*PAGE_SIZE);
			printf("temp virtual page: 0x%x\n", (uint32_t) temp_virtual);
			if (pagemngr_map_block(physical,temp_virtual) == -1) {
				printf("could not map temporary block\n");
				return NULL;
			}
			_temp_page_bitmap |= (uint8_t) (1 << i);
			return temp_virtual+physical_offset;			// make sure we return into the offset of the page
		}
	}

	// no free temp page; return null
	printf("no free temp page\n");
	return NULL;
}

void *memory_release_temp_map(void *virtual) {
	// make sure the virtual address is within the temporary mappings
	uintptr_t address_val = (uintptr_t) virtual;
	if (address_val < FIRST_TEMPORARY_PAGE || address_val > LAST_TEMPORARY_PAGE) {
		errno = EINVAL;
		return NULL;
	}

	void *physical = pagemngr_unmap_block(virtual);
	if (physical) {
		size_t index = (size_t)(address_val - FIRST_TEMPORARY_PAGE) / PAGE_SIZE;
		_temp_page_bitmap &= ~(uint8_t)(1 << index);
	}
	return physical;
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
				i--;
			}
			errno = ENOMEM;
			return -1;
		}
		if (pagemngr_map_block(block,map_address) == -1) {
			while (i > 0) {
				void *physical = pagemngr_unmap_block(map_address);
				if (physical) {
					balloc_deallocate_block(physical);
					map_address -= PAGE_SIZE;
				}
				i--;
			}
			return -1;
		}
		map_address += PAGE_SIZE;
	}
	return 0;
}