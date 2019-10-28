/**
 * string/other.c: provides other non-categorizable string.h functions
 */

#include <string.h>

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
	size_t i = 0;
	for (; str[i] != '\0'; i++) ;
	return i;
}