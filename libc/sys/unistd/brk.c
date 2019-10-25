/**
 * sys/unistd/brk.c: implements brk / sbrk functionality
 */

#include <kernel/memory.h>

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <errno.h>

#if defined(__build_i386)
#define PAGE_SIZE		4096
#else
#define PAGE_SIZE		1
#endif

#define ALIGN_DOWN(x)	(x &~ (PAGE_SIZE-1))
#define ALIGN_UP(x)		(ALIGN_DOWN(x)+PAGE_SIZE)

static void *_brk = NULL;
static void *_base = NULL;

/**
 * Set the new program break (end of heap space for this program)
 * This should be called at the start of each process to set the
 * base data segment
 */
int brk(void *addr) {

	// first brk allocates a single page at the requested break address
	if (!_base) {
		_base = addr;
		if (memory_get_blocks(addr,1) == -1) {
			errno = ENOMEM;
			return -1;
		}
		_brk = addr;
		return 0;
	}

	// make sure we aren't trying to set the break to some
	// undefined region of memory before the heap start
	uintptr_t __heap_base_addr = (uintptr_t)(_base);
	uintptr_t __new_brk_addr = (uintptr_t)(addr);

	if (__new_brk_addr < __heap_base_addr) {
		errno = ENOMEM;
		return -1;
	}

	// page-align the new brk addr to the next page up
	uintptr_t __page_aligned_top = ALIGN_UP(__new_brk_addr);

	// fresh allocation 
	if (!_brk) {
		size_t block_count = (__page_aligned_top-__heap_base_addr) / PAGE_SIZE;
		void *frame_addr = (void *) __heap_base_addr;

		// allocate and map blocks until block_count is satisifed
		if (memory_get_blocks(frame_addr, block_count) == -1) return -1;
		_brk = addr;

	} else {
		uintptr_t __old_brk_addr = (uintptr_t) _brk;

		// new break is inside the last mapped page
		if (ALIGN_DOWN(__new_brk_addr) == ALIGN_DOWN(__old_brk_addr)) {
			_brk = addr;
		} else {
			size_t new_block_count = ((__page_aligned_top-ALIGN_DOWN(__old_brk_addr)) / PAGE_SIZE) - 1;
			void *frame_addr = (void *) ALIGN_UP(__old_brk_addr);

			if (memory_get_blocks(frame_addr,new_block_count) == -1) return -1;
			_brk = addr;
		}
	}
	return 0;
}

void *sbrk(intptr_t increment) {
	void *_old_brk = _brk;
	if (increment == 0) {
		return _brk;
	}

	if (brk((void *)_brk+increment) != 0) {
		errno = ENOMEM;
		return (void *)(-1);
	}

	return _old_brk;
}