#ifndef _KERNEL_ITOA_H
#define _KERNEL_ITOA_H 1

#include <stdint.h>
#include <stddef.h>

// minimum safe buffer size for itoa and itoa_s
#define ITOA_BUFFER_SIZE	33

#ifdef __cplusplus
extern "C" {
#endif

int itoa(char *destination, unsigned int num, int base);
int itoa_s(char *destination, int num, int base);

#ifdef __cplusplus
}
#endif

#endif // _KERNEL_ITOA_H