#include <init/kparams.h>

#include <kernel/display.h>
#include <kernel/cpu.h>
#include <kernel/version.h>
#include <kernel/itoa.h>
#include <kernel/timer.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>

char number_buffer[ITOA_BUFFER_SIZE];

void kernel_main(_kernel_params_t *kparams) {
	display_init();
	display_change_color(display_make_color(COLOR_FG_WHITE,COLOR_BG_GREEN));
	display_clear();

	const char *kernel_name = KERNEL_NAME_STRING;
	const char *kernel_version = KERNEL_VERSION_STRING;

	puts("Initializing kernel...");
	printf("Kernel %s, version %s\n", kernel_name, kernel_version);

	cpu_driver_init();
	unsigned int old_timer_value = pit_8254_get_ticks();

	while (1) {
		unsigned int new_timer_value = pit_8254_get_ticks();
		if (new_timer_value != old_timer_value) {
			printf("tick: %u\n", new_timer_value);
			old_timer_value = new_timer_value;
		}
	}
}