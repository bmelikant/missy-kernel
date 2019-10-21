/**
 * stdio/vprintf.c: Provide stdio vprintf function for the kernel
 * TODO: This version of vprintf is NOT standards-compliant. Update this function
 * to comply with standard C printf formatting rules
 */

#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <limits.h>
#include <errno.h>

#if defined(__is_libk)
#include <kernel/itoa.h>
#endif

#ifndef __cplusplus
#include <stdbool.h>
#endif

static bool print(const char *str, size_t length) {
	while (length > 0) {
		if (putchar(*str++) == EOF) {
			return false;
		}
		length--;
	}
	return true;
}

/**
 * This code was taken from the Meaty Skeleton project at
 * https://wiki.osdev.org/Meaty_Skeleton#libc.2Fstdio.2Fprintf.c
 * Adapted to be a vprintf implementation and to include integer support
 */
int vprintf(const char* restrict fmt, va_list arguments) {
	int written = 0;
	while (*fmt != '\0') {
		size_t maxrem = INT_MAX - written;
		if (fmt[0] != '%' || fmt[1] == '%') {
			// print regular string data from this format string
			if (fmt[0] == '%') {
				// TODO: I don't think this will ever get executed!!! check into this
				fmt++;
			}
			size_t amount = 1;
			while (fmt[amount] && fmt[amount] != '%') {
				amount++;
			}
			if (maxrem < amount) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(fmt,amount)) {
				return -1;
			}

			fmt += amount;
			written += amount;
			continue;
		}

		const char *fmt_begun_at = fmt++;

		if (*fmt == 'c') {
			fmt++;
			char c = (char) va_arg(arguments, int);		// char promotes to int
			if (!maxrem) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(&c,sizeof(char))) {
				return -1;
			}
			written++;
		} else if (*fmt == 's') {
			fmt++;
			const char *str = va_arg(arguments, const char*);
			size_t length = strlen(str);
			if (maxrem < length) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(str, length)) {
				return -1;
			}
			written += length; 
		} else if (*fmt == 'x' || *fmt == 'X' || *fmt == 'o' || *fmt == 'u') {
			int base = 10;
			
			if (*fmt == 'x' || *fmt == 'X') base = 16;
			else if (*fmt == 'o') base = 8;

			fmt++;
			unsigned int num = va_arg(arguments, unsigned int);
			#if defined(__is_libk)
			char buffer[ITOA_BUFFER_SIZE];
			size_t length = itoa(buffer,num,base);
			if (maxrem < length) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(buffer,length)) {
				return -1;
			}
			written += length;
			#else
			// TODO: add userspace snprintf logic here
			#endif
		} else if (*fmt == 'd' || *fmt == 'i') {
			fmt++;
			int num = va_arg(arguments, int);
			#if defined(__is_libk)
			char buffer[ITOA_BUFFER_SIZE];
			size_t length = itoa(buffer,num,10);
			if (maxrem < length) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(buffer,length)) {
				return -1;
			}
			written += length;
			#else
			// TODO: add userspace snprintf logic here
			#endif
		} 
		#if defined(__is_libk)		// print binary formats for the kernel
		else if (*fmt == 'b') {
			fmt++;
			unsigned int num = va_arg(arguments, unsigned int);
			char buffer[ITOA_BUFFER_SIZE];
			size_t length = itoa(buffer,num,2);
			if (maxrem < length) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(buffer,length)) {
				return -1;
			}
			written += length;
		}
		#endif
		else {
			fmt = fmt_begun_at;
			size_t length = strlen(fmt);
			if (maxrem < length) {
				errno = EOVERFLOW;
				return -1;
			}
			if (!print(fmt,length)) {
				return -1;
			}
			written += length;
			fmt += length;
		}
	}
	return written;
}