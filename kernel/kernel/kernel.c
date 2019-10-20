#include <init/kparams.h>
#include <kernel/display.h>
#include <stdio.h>

void kernel_main(_kernel_params_t *kparams) {
	display_init();
	display_change_color(display_make_color(COLOR_FG_WHITE,COLOR_BG_GREEN));
	display_clear();
	puts("welcome to the kernel!!!");
}