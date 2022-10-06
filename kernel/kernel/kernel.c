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

__attribute__((noinline,section(".testmethod"))) void test_method_2() {
	printf("I am in another new process!\n");
	return;
}

//extern void enter_usermode(void *fn);

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

void user_puts(char *s) {
	__asm__ volatile(
		"xor %%eax,%%eax\n\t"
		"lea (%0),%%ebx\n\t"
		"int $0x80\n\t"
		:: "b"(s)
	);
}

void print_int(char *s, int i) {
	__asm__ volatile(
		"movl $0x02,%%eax\n\t"
		"lea (%0),%%ebx\n\t"
		"movl %1,%%ecx\n\t"
		"int $0x80\n\t"
		:: "b"(s),"g"(i)
	);
}

int spawn_process(void *method_address) {
	int pid = 0;
	__asm__ volatile(
		"movl $0x01,%%eax\n\t"
		"lea (%1),%%ebx\n\t"
		"movl $0x01,%%ecx\n\t"
		"movl $0x01,%%edx\n\t"
		"int $0x80\n\t"
		"movl %%eax,%0"
		: "=m"(pid)
		: "b"(method_address)
	);
	return pid;
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

	// attempt to jump into user mode since the system is "dead" anyway
	enter_usermode();

	/** okay let's test creating a process. Probably gonna go bad... **/
	//for (int i = 0; i < 10; i++) {
		int process_id = spawn_process(test_method);
		char *pid_string = "PID: %d\n";
		print_int(pid_string, process_id);
	//}

	int process_id_2 = spawn_process(test_method_2);
	print_int(pid_string, process_id_2);

/*
	process_id = spawn_process(test_method);
	print_int(pid_string, process_id);

	process_id = spawn_process(test_method);
	print_int(pid_string, process_id);
*/

	user_puts("Well, made it past that method up there.");

	for (;;);
}