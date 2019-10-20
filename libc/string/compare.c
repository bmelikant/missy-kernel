/**
 * libc/string/compare.c: Provide string and memory comparison operations for the standard C runtime
 * Since most of the string routines are small, I am trying to reduce bloat by grouping
 * items together that have similar functionality
 */
#include <string.h>

/**
 * Compare two blocks of memory for equality
 */
int memcmp(const void *mem1, const void *mem2, size_t len) {
	const char *char1 = (const char *) mem1;
	const char *char2 = (const char *) mem2;
	while (len > 0) {
		if (*char1 != *char2) {
			break;
		}
		char1++;char2++;
	}
	return (int)(*char1-*char2);
}

/**
 * Compare two strings. Don't take into account any length, just
 * read until the first null character is encountered in either string
 */
int strcmp(const char *str1, const char *str2) {
	while (*str1 && *str2) {
		if (*str1 != *str2) {
			break;
		}
		str1++;str2++;
	}
	return (int)(*str1-*str2);
}

/**
 * Compare two strings, taking into account the provided length parameter
 * If the null terminator is reached for either string before the length is
 * used up, the string read will stop
 */
int strncmp(const char *str1, const char *str2, size_t length) {
	while (*str1 && *str2 && length > 0) {
		if (*str1 != *str2) {
			break;
		}
		str1++;str2++;length--;
	}
	return (int)(*str1-*str2);
}