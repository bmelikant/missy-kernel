#include <init/kutils.h>
#include <init/kerrno.h>
#include <init/kterm.h>

#ifndef _cplusplus
#include <stdbool.h>
#endif

void *ki_memset(void *dst, int c, size_t count) {
	char *destination = (char *) dst;
	for (size_t i = 0; i < count; i++) {
		destination[i] = c;
	}
	return dst;
}

void *ki_memcpy(void *dst, const void *src, size_t count) {
	char *destination = (char *) dst;
	const char *source = (const char *) src;
	for (size_t i = 0; i < count; i++) {
		destination[i] = source[i];
	}
	return dst;
}

size_t ki_strlen(const char *str) {
	char *s = (char *) str;
	while (*s++);
	return (size_t) (s-str)-1;
}

static const char lettermap [] = "0123456789abcdef";
static void zero_string(char *buf) {
	buf[0] = '0';
	buf[1] = '\0';
}

void ki_make_ulong_string(char *buffer, unsigned long number, int base) {
	// zero out the buffer string
	ki_memset(buffer,0,MAXCHARS);
	if (number == 0) {
		zero_string(buffer);
		return;
	}
	if (base > 16 || base < 2) {
		kinit_errno = EINVAL;
		return;
	}

	// we want to point at the first available character. preserve the terminating null character
	char *writeptr = (char *)(buffer+(MAXCHARS-1));
	while (number > 0) {
		*(--writeptr) = lettermap[number % base];
		number /= base;
	}

	while ((*buffer++ = *writeptr++)) ;
}