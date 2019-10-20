#ifndef _LIBC_STDIO_H
#define _LIBC_STDIO_H

#include <stdarg.h>

int putchar(int);						// putchar.c, done
int puts(const char *);					// puts.c, done
int printf(const char *,...);
int vprintf(const char *,va_list);

#endif