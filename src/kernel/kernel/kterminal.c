#include <kernel/kterminal.h>

#ifdef __build_i386
#include <kernel/driver/i386/display.h>
#include <kernel/driver/i386/kbinput.h>
#endif

int kterm_write_string(const char *str) {
	
}