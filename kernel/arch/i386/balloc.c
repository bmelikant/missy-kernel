/**
 * Kernel space physical block allocator
 * Return allocated blocks of physical memory
 */

#include "balloc.h"

#include <stdio.h>

#include <stddef.h>
#include <stdint.h>
#include <errno.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#define BALLOC_PAGE_SIZE 4096
#define BLOCKS_PER_UNIT	 32
#define FULL_UNIT		 0xffffffff

#define block_is_allocated(x)	(_mbmp[x/BLOCKS_PER_UNIT])&(1<<(x%BLOCKS_PER_UNIT))

uint32_t *_mbmp = NULL;
static unsigned int _total_blocks = 0;
static unsigned int _used_blocks = 0;

static unsigned int first_free() {
	for (size_t i = 0; i < _total_blocks / BLOCKS_PER_UNIT; i++) {
		if (_mbmp[i] != FULL_UNIT) {
			for (size_t j = 0; j < BLOCKS_PER_UNIT; j++) {
				if (((_mbmp[i] >> j) & 1) == 0) {
					return (i*BLOCKS_PER_UNIT)+j;
				}
			}
		}
	}
	errno = ENOMEM;
	return 0;
}

static inline void allocate(unsigned int block) {
	unsigned int i = block / BLOCKS_PER_UNIT;
	unsigned int j = block % BLOCKS_PER_UNIT;
	_mbmp[i] |= (1<<j);
	_used_blocks++;
}

static inline void deallocate(unsigned int block) {
	unsigned int i = block / BLOCKS_PER_UNIT;
	unsigned int j = block % BLOCKS_PER_UNIT;
	_mbmp[i] &= ~(1<<j);
	_used_blocks--;
}

void balloc_initialize(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks) {
	_mbmp = mbmp;
	_total_blocks = total_blocks;
	_used_blocks = used_blocks;
}

void *balloc_allocate_block() {
	uint32_t free_block = first_free();
	if (free_block == 0) {
		return NULL;
	}

	allocate(free_block);
	void *allocated_ptr = (void *)(free_block*BALLOC_PAGE_SIZE);
	printf("Address of new block: 0x%x\n", (uint32_t) allocated_ptr);
	
	return allocated_ptr;
}

void balloc_deallocate_block(void *block) {
	uint32_t allocated_block = (uint32_t)(block) / BALLOC_PAGE_SIZE;

	if (block_is_allocated(allocated_block)) {
		// make sure the block isn't outside the block map
		if (allocated_block < _total_blocks) {
			deallocate(allocated_block);
		}
	}
}

unsigned int balloc_get_total_blocks() {
	return _total_blocks;
}

unsigned int balloc_get_used_blocks() {
	return _used_blocks;
}

unsigned int balloc_get_free_blocks() {
	return (_total_blocks - _used_blocks);
}