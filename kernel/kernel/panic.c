/**
 * Initiate a kernel panic
 * Panic prints the given panic message then halts the system
 */

#include <kernel/panic.h>
#include <kernel/display.h>
#include <stdio.h>

#define KERNEL_PANIC_MESSAGE	"*** KERNEL PANIC ***\nThe kernel initiated a panic for the following cause:\n"
#define die()	for(;;);

__attribute__((__noreturn__))
void kernel_panic(const char *cause) {
	display_change_color(display_make_color(COLOR_FG_WHITE,COLOR_BG_RED));
	display_clear();

	puts(KERNEL_PANIC_MESSAGE);
	puts(cause);

	die();
	__builtin_unreachable();
}