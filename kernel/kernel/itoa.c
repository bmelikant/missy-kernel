/**
 * These two routines allow the kernel to convert numbers to strings
 * The macro ITOA_BUFFER_SIZE defines the minimum safe buffer size to these
 * functions
 */

#include <kernel/itoa.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

static const char lettermap[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

int itoa(char *buffer, unsigned int num, int base) {
	if (base > 36 || base < 2) {
		errno = ERANGE;
		*buffer = '\0';
		return 0;
	}

	if (num == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	memset(buffer,0,ITOA_BUFFER_SIZE);
	char *writeptr = (char *)(buffer+(ITOA_BUFFER_SIZE-1));
	int written = 0;

	while (num > 0) {
		writeptr--;
		*writeptr = lettermap[num%base];
		num /= base;
		written++;
		if (written > ITOA_BUFFER_SIZE-1) {
			errno = EOVERFLOW;
			return -1;
		}
	}

	// copy the number to the front of the string
	while ((*buffer++ = *writeptr++)) ;
	return written;
}

int itoa_s(char *buffer, int num, int base) {
	bool negative = false;
	if (num < 0) {
		num = -num;
		negative = true;
	}

	if (base > 36 || base < 2) {
		errno = ERANGE;
		*buffer = '\0';
		return 0;
	}

	if (num == 0) {
		buffer[0] = '0';
		buffer[1] = '\0';
		return 1;
	}

	memset(buffer,0,ITOA_BUFFER_SIZE);
	char *writeptr = (char *)(buffer+(ITOA_BUFFER_SIZE-1));
	int written = 0;

	while (num > 0) {
		writeptr--;
		*writeptr = lettermap[num%base];
		num /= base;
		written++;

		if (written >= ITOA_BUFFER_SIZE-2) {
			errno = EOVERFLOW;
			return -1;
		}
	}

	*buffer++ = '-';
	// copy the number to the front of the string
	while ((*buffer++ = *writeptr++)) ;
	return written;
}