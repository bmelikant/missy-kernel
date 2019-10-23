/**
 * Adding this library so that my system can support the POSIX standard
 * There is a TON of stuff in this header...
 */

#include <stddef.h>
#include <stdint.h>

int	  brk(void *addr);
void *sbrk(intptr_t increment);