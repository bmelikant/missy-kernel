#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif

int putchar(int);						// putchar.c, done
int puts(const char *);					// puts.c, done

#if defined(__is_libk) || defined(__is_missy_kernel)
int printf(const char *,...);			// stdio/printf.c for kernel, macro to fprintf for libc
int vprintf(const char *,va_list);		// stdio/vprintf.c for kernel, macro to fvprintf for libc
#else
// TODO: provide macro versions for libk
#endif

#ifdef __cplusplus
}
#endif

#endif // _LIBC_STDIO_H