#include <kparams.h>
#include <kernel/display.h>

void kernel_main(_kernel_params_t *kparams) {
	display_init();
	char *hello = "Hello, world!\n";
	while (*hello) {
		display_write(*hello++);
	}
}