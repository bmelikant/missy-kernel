extern void puts();
extern void create_process();
extern void printf();

#define MAX_SYSCALLS 3
void *syscalls[] = {
    puts,
    create_process,
    printf
};

#ifdef __build_i386
__attribute__((naked)) void syscall(void) {
	static int idx = 0;
	__asm__ volatile(
		"movl %%eax,%0"
		: "=m"(idx)
	);

	if (idx >= MAX_SYSCALLS)
	__asm__ volatile("iret");

	void *fn = syscalls[idx];

	__asm__ volatile(
		"push %%edi\n\t"
		"push %%esi\n\t"
		"push %%edx\n\t"
		"push %%ecx\n\t"
		"push %%ebx\n\t"
		"call %0\n\t"
		"add $20,%%esp\n\t"
		"iret\n\t"
		:: "r"(fn)
	);
}
#endif