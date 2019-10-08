#include <init/kutils.h>

void *early_memset(void *dst, int c, size_t count) {
	char *destination = (char *) dst;
	for (size_t i = 0; i < count; i++) {
		destination[i++] = c;
	}
	return dst;
}

const char 	lettermap [] = "0123456789abcdef";
char		buffer[32];
extern int 	errno;

size_t early_strlen(const char *str) {
	char *s = (char *) str;
	while (*s++);
	return (size_t) (s-str)-1;
}

char *early_itoa_s(char *str, int num, int base) {
	int pos  = 0;
	int opos = 0;
	int top  = 0;

	// base must be < 16 and i cannot be zero
	if (num == 0 || base > 16) {

		str[0] = '0';
		str[1] = '\0';

		return str;
	}

	// now transform the number to a string
	while (num > 0) {

		buffer[pos] = lettermap[num % base];
		pos++;
		num /= base;
	}

	top = pos--;

	for (opos = 0; opos < top; pos--,opos++)
		str[opos] = buffer[pos];

	str[opos] = 0;
	return str;
}