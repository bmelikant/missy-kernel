/**
 * stdio/printf.c
 * print a formatted string to stdout
 */

#include <stdio.h>
#include <stdarg.h>

/**
 * This function will be called from the kernel
 */
int printf(const char *fmt, ...) {
	va_list arguments;
	va_start(arguments,fmt);
	int result = vprintf(fmt,arguments);
	va_end(arguments);
	return result;
}