#include <init/kparams.h>

#include <kernel/display.h>
#include <kernel/cpu.h>
#include <kernel/version.h>
#include <kernel/itoa.h>
#include <kernel/timer.h>
#include <kernel/memory.h>
#include <kernel/serial.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/unistd.h>

void kernel_main(_kernel_params_t *kparams) {
	display_init();
	display_change_color(display_make_color(COLOR_FG_WHITE,COLOR_BG_GREEN));
	display_clear();

	printf("%s\n", banner());

	cpu_driver_init();
	pit_8254_initialize();

	// initialize kernel memory management
	memory_setbase(kparams->kernel_heap);
	memory_init_mmap(kparams->kernel_memory_bitmap, kparams->allocator_total_blocks, kparams->allocator_used_blocks);
	brk((void *)(kparams->kernel_heap));

	int dd = serial_init(COM_PORT_1);
	printf("com port device descriptor: %d\n", dd);

	int c = 0;
	while ((c = read_device_char(dd)) != -1) {
		putchar(c);
	}

	printf("Received EOF from input stream\n");
}