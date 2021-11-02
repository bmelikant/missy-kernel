/**
 * string/other.c: provides other non-categorizable string.h functions
 */

#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/bochsdbg.h>

/**
 * initialize a block of memory with the given character
 */
void *memset(void *destination, int initial, size_t count) {
	char *dest = (char *) destination;
	while (count > 0) {
		*dest++ = (char) initial;
		count--;
	}
	return destination;
}

/**
 * get the length of a null-terminated c string
 */
size_t strlen(const char *str) {
	const char *s2 = str;
	while (*s2++) ;
	return (size_t)(s2-str)-1;
}