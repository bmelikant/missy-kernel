#ifndef _KERNEL_MEMORY_MANAGER_H
#define _KERNEL_MEMORY_MANAGER_H 1

#include <stdint.h>
#include <stddef.h>

#if defined(__build_i386)
#define PAGE_SIZE			4096
#define BLOCK_ALIGN			4096
#define ALIGN_BLOCK_UP(x)	((x &~ 0xfff)+PAGEMNGR_PAGE_SIZE)
#define ALIGN_BLOCK_DOWN(x)	(x &~ 0xfff)
#endif

void memory_init_mmap(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks);
int  memory_get_blocks(void *requested_addr, size_t count);

#endif // _KERNEL_MEMORY_MANAGER_H