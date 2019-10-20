#ifndef _LIBC_STDLIB_H
#define _LIBC_STDLIB_H

#include <sys/cdefs.h>

#ifdef __cplusplus
extern "C" {
#endif

__attribute__((__noreturn__))
void abort(void);					// stdlib/abort.c

#ifdef __cplusplus
}
#endif

#endif // _LIBC_STDLIB_H