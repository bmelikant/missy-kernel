#ifndef _KERNEL_ARCH_I386_BALLOC_H
#define _KERNEL_ARCH_I386_BALLOC_H 1

#include <stdint.h>

void balloc_initialize(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks);
void *balloc_allocate_block();
void balloc_deallocate_block(void *block);

unsigned int balloc_get_total_blocks();
unsigned int balloc_get_used_blocks();
unsigned int balloc_get_free_blocks();

#endif // _KERNEL_ARCH_I386_BALLOC_H