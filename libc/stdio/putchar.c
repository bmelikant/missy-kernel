#include <stdio.h>

#if defined(__is_libk)
#include <kernel/display.h>
#endif // defined(__is_libk)

int putchar(int c) {
	#if defined(__is_libk)
	display_write(c);
	#else
	// TODO: add libc write logic
	#endif
	return c;
}