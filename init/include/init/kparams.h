#ifndef _KERNEL_PARAMETERS_H
#define _KERNEL_PARAMETERS_H

#include <stdint.h>

struct KERNEL_PARAMETERS {
	uint32_t *kernel_stack;
	uint32_t *kernel_heap;
	uint32_t *kernel_memory_bitmap;
	uint32_t bitmap_size;
};

typedef struct KERNEL_PARAMETERS _kernel_params_t ;

#endif