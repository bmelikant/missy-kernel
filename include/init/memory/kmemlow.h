#ifndef _PHYS_LOW_H
#define _PHYS_LOW_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t* physaddr_p;
typedef uint32_t* virtaddr_p;

int kmemlow_init_allocator();       // initialize the physical memory manager
void *kmemlow_alloc();              // allocate a single block of memory
void  kmemlow_free(void *);         // free a single block of memory

unsigned int kmemlow_get_total_blocks();
unsigned int kmemlow_get_free_blocks();
unsigned int kmemlow_get_used_blocks();

/** 
 * API note:
 * These functions allow the kernel to allocate contiguous blocks of free memory
 * THE CALLER MUST KEEP TRACK OF WHICH POINTERS HAVE MULTIPLE BLOCKS ALLOCATED!!
 * Freeing blocks of memory allocated for other purposes may have unintended side
 * effects on subsequent requests
 */
void *kmemlow_alloc_s(size_t count);
void  kmemlow_free_s(void *block, size_t count);

#endif