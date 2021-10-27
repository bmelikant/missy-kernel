#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include <sys/cdefs.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define __ptr_t void *

__attribute__((__noreturn__))
void abort(void);					// stdlib/abort.c

/** malloc and friends */
void *malloc(size_t size);
void *calloc(size_t count, size_t size);
void *realloc(void *ptr, size_t size);
void free(void *ptr);

#ifdef __cplusplus
}
#endif

#endif // _LIBC_STDLIB_H