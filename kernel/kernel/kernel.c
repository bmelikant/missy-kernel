#include <init/kparams.h>

#include <kernel/display.h>
#include <kernel/cpu.h>
#include <kernel/version.h>
#include <kernel/itoa.h>
#include <kernel/timer.h>
#include <kernel/memory.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <sys/unistd.h>

char number_buffer[ITOA_BUFFER_SIZE];

void kernel_main(_kernel_params_t *kparams) {
	display_init();
	display_change_color(display_make_color(COLOR_FG_WHITE,COLOR_BG_GREEN));
	display_clear();

	const char *kernel_name = KERNEL_NAME_STRING;
	const char *kernel_version = KERNEL_VERSION_STRING;

	puts("Initializing kernel...");
	printf("Kernel %s, version %s\n\n", kernel_name, kernel_version);

	cpu_driver_init();
	pit_8254_initialize();

	// display the kernel paramters. this should make it easier for me
	// to figure out how to write my initial brk()
	printf("Kernel Parameters struct:\n");
	printf("kparams->kernel_stack = 0x%x\n", (unsigned int) kparams->kernel_stack);
	printf("kparams->kernel_heap = 0x%x\n", (unsigned int) kparams->kernel_heap);
	printf("kparams->memory_bitmap = 0x%x\n", (unsigned int) kparams->kernel_memory_bitmap);
	printf("kparams->allocator_total_blocks = %u\n", kparams->allocator_total_blocks);
	printf("kparams->allocator_used_blocks = %u\n", kparams->allocator_used_blocks);

	// allocate the first page beyond the end of the kernel to start the kernel heap
	memory_setbase(kparams->kernel_heap);
	memory_init_mmap(kparams->kernel_memory_bitmap, kparams->allocator_total_blocks, kparams->allocator_used_blocks);

	brk((void *)(kparams->kernel_heap)+0xfff);
}