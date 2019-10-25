#ifndef _KERNEL_MEMORY_MANAGER_H
#define _KERNEL_MEMORY_MANAGER_H 1

#include <stdint.h>
#include <stddef.h>

void memory_init_mmap(uint32_t *mbmp, uint32_t total_blocks, uint32_t used_blocks);
void *memory_request_temp_map(void *physical);
void *memory_release_temp_map(void *virtual);
int  memory_get_blocks(void *requested_addr, size_t count);

#endif // _KERNEL_MEMORY_MANAGER_H