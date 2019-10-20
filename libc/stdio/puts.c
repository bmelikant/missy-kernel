#include <stdio.h>

int puts(const char *str) {
	const char *s2 = str;
	while (*s2) {
		putchar(*s2++);
	}
	putchar('\n');
	return (int)(s2-str)+1;	// +1 for the terminating newline
}