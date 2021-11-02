#include <init/kparams.h>

#include <kernel/display.h>
#include <kernel/cpu.h>
#include <kernel/version.h>
#include <kernel/itoa.h>
#include <kernel/timer.h>
#include <kernel/memory.h>
#include <kernel/serial.h>
#include <kernel/process.h>

#include <kernel/bochsdbg.h>

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include <sys/unistd.h>

__attribute__((noinline,section(".testmethod"))) void test_method() {
	printf("I am in a new process!\n");
	return;
}

extern unsigned int testmethod_end;
extern unsigned int testmethod_start;

void *testmethod_start_ptr = &testmethod_start;
void *testmethod_end_ptr = &testmethod_end;

int timer_test_method(void) {
	printf("Hello, timer world!\n");
	return 0;
}

int userspace_method() {
	//printf("Hello, user space world!\n");
	puts("hello");
	for(;;);
	return 0;
}

static inline void flush_pdbr() {
	__asm__(
		"mov %eax,%cr3\n\t"
		"mov %cr3,%eax\n\t"
	);
}

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

	int pid = create_process((__ptr_t)(test_method), (testmethod_end_ptr - testmethod_start_ptr),4096);
	printf("Successfully created process %d of %i bytes\n", pid, (testmethod_end_ptr - testmethod_start_ptr));

	start_process(pid);
	
	int dd = serial_init(COM_PORT_1);
	printf("com port device descriptor: %d\n", dd);

	//int c = 0;
	//while ((c = read_device_char(dd)) != -1) {
	//	putchar(c);
	//}

	printf("Received EOF from input stream\n");

	//flush_pdbr();

	// attempt to jump into user mode since the system is "dead" anyway
	enter_usermode();

	//BOCHS_MAGIC_BREAKPOINT();

/*
	char *teststr = "Winky desserts, world!\n";
	__asm__(
		"xor %%eax,%%eax\n\t"
		"lea (%0),%%ebx\n\t"
		"int $0x80\n\t"
		:: "b"(teststr)
	);

	char *teststr2 = "Hallo, world!\n";
	__asm__(
		"xor %%eax,%%eax\n\t"
		"lea (%0),%%ebx\n\t"
		"int $0x80\n\t"
		:: "b"(teststr2)
	);
*/

	for (;;);
}