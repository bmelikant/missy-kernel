#ifndef _PHYS_LOW_H
#define _PHYS_LOW_H

#include <stddef.h>
#include <stdint.h>

#define KMEMLOW_ERROR_ENOMEM	0x01

int kmemlow_init_allocator();       // initialize the physical memory manager
void *kmemlow_alloc();              // allocate a single block of memory
void  kmemlow_free(void *);         // free a single block of memory
void *kmemlow_get_kernel_endptr();	// get a pointer to the first 4kb aligned block above the kernel
void *kmemlow_get_bitmap_ptr();		// return the physical pointer to the memory bitmap
uint32_t kmemlow_get_bitmap_size();	// get the size of the memory bitmap so the kernel can set itself up later

unsigned int kmemlow_get_total_blocks();
unsigned int kmemlow_get_free_blocks();
unsigned int kmemlow_get_used_blocks();

#endif