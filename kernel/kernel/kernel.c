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
#include <string.h>
#include <sys/unistd.h>

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

	// allocate the first page beyond the end of the kernel to start the kernel heap. set the kernel break
	// to the first page beyond the kernel
	memory_setbase(kparams->kernel_heap);
	memory_init_mmap(kparams->kernel_memory_bitmap, kparams->allocator_total_blocks, kparams->allocator_used_blocks);
	brk((void *)(kparams->kernel_heap));

	char *test = (char *) malloc(200);
	char *test2 = (char *) malloc(200);

	char *testline = "Hello, world!\n";
	printf("test -> 0x%x\n", (unsigned int) test);
	printf("test2 -> 0x%x\n", (unsigned int) test2);
	memset(test,0,200);
	strncpy(test,testline,strlen(testline));
	printf("testline -> %s\n",test);
}