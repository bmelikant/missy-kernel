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
	printf("Kernel %s, version %s\n\n", kernel_name, kernel_version);
	
	printf("Kernel parameters:\n");
	printf("kparams->kernel_stack = 0x%x\n", (uint32_t)(kparams->kernel_stack));
	printf("kparams->kernel_heap = 0x%x\n", (uint32_t)(kparams->kernel_heap));
	printf("kparams->kernel_memory_bitmap = 0x%x\n", (uint32_t)(kparams->kernel_memory_bitmap));
	printf("kparams->bitmap_size = %d\n", kparams->bitmap_size);
	printf("kparams->rsdt_address = 0x%x\n\n",(uint32_t)(kparams->rsdt_address));

	cpu_driver_init();
	pit_8254_initialize();
}