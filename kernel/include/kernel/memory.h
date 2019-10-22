#ifndef _KERNEL_MEMORY_MANAGER_H
#define _KERNEL_MEMORY_MANAGER_H 1

#include <stdint.h>
#include <stddef.h>

void memory_setbase(void *base_address);
void memory_init_mmap(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks);

int brk(void *addr);
void *sbrk(intptr_t increment);

#endif // _KERNEL_MEMORY_MANAGER_H