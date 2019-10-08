#ifndef _MEMORY_H
#define _MEMORY_H

#include <stddef.h>
#include <stdint.h>

typedef uint32_t* physaddr_p;
typedef uint32_t* virtaddr_p;

int kmemory_init_allocator();

#endif