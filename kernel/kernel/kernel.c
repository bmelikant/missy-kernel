#include <init/kparams.h>

#include <kernel/display.h>
#include <kernel/version.h>
#include <kernel/itoa.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

char number_buffer[ITOA_BUFFER_SIZE];

void kernel_main(_kernel_params_t *kparams) {
	display_init();
	display_change_color(display_make_color(COLOR_FG_WHITE,COLOR_BG_GREEN));
	display_clear();
	
	puts("Welcome to the kernel! Version information:");
	puts(KERNEL_NAME);
	puts(KERNEL_VERSION_STRING);

	char *test_string = "Hello, world!";
	printf("string: %s\n",test_string);
}