#ifndef _KERNEL_EARLY_UTILS_H
#define _KERNEL_EARLY_UTILS_H

#include <stddef.h>

void early_memset(void *dst, int c, size_t len);

#endif