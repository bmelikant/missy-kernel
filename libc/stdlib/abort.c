/**
 * stdlib/abort.c
 * Abort the currently running application
 * When called in the kernel this function currently induces a kernel panic
 * Eventually this should cause the kernel to shut down gracefully
 */

#ifdef __is_libk
#include <kernel/panic.h>
#endif

#define die_on_abort() for(;;);

__attribute__((__noreturn__))
void abort(void) {
#ifdef __is_libk
	kernel_panic("abort() was called");
#else
#endif
	die_on_abort();
	__builtin_unreachable();
}