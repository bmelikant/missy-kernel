/**
 * libc/string/movecopy.c: Provide move/copy operations for the C standard string library
 * Since most of the string routines are small, I am trying to reduce bloat by grouping
 * items together that have similar functionality
 */
#include <string.h>


/**
 * Copy a block of memory to another block of memory
 */
void *memcpy(void *destination, const void *source, size_t len) {
	char *dst = (char *) destination;
	const char *src = (const char *) source;

	while (len > 0) {
		*dst++ = *src++;
		len--;
	}
	return destination;
}

/**
 * Copy a string to a string. Not string-length safe version
 */
char *strcpy(char *destination, const char *source) {
	char *dst = destination;
	while (*source) {
		*dst++ = *source++;
	}
	return destination;
}

/**
 * Copy a string to a string. String-length safe version since length is specified
 */
char *strncpy(char *destination, const char *source, size_t length) {
	char *dst = destination;
	while (length > 0) {
		*dst++ = *source++;
		length--;
	}
	return destination;
}