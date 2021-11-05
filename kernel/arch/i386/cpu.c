#include <kernel/cpu.h>
#include <kernel/pic.h>
#include <kernel/timer.h>
#include <kernel/bochsdbg.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <cpuid.h>
#include <stddef.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

#ifndef __build_i386
#error "Cannot build cpu.c driver for non-i386 systems"
#endif

#define GDT_MAX 6
#define MAX_INTERRUPTS 256

#define INTERRUPT_TYPE_PRESENT		0x80
#define INTERRUPT_TYPE_SYSTEM		0x00
#define INTERRUPT_TYPE_USER			0x60
#define INTERRUPT_TYPE_GATE32		0x0E

#define DEVICE_INTERRUPT_BASE	0x20

#define INTERRUPT32_TYPE		(INTERRUPT_TYPE_PRESENT|INTERRUPT_TYPE_SYSTEM|INTERRUPT_TYPE_GATE32)
#define SYSCALL_INTERRUPT_TYPE	(INTERRUPT_TYPE_PRESENT|INTERRUPT_TYPE_USER|INTERRUPT_TYPE_GATE32)
#define SYSTEM_CODE_SELECTOR	0x08

#define CPUID_BASIC 	0x0
#define CPUID_FEATURES	0x00000001
#define CPUID_EXTENDED 	0x80000000

#define CPUID_FEATURE_FPU_PRESENT	(1<<0)
#define CPUID_FEATURE_APIC_PRESENT 	(1<<9)

typedef void(*isr_handler)();

typedef struct GLOBAL_DESCRIPTOR_TABLE {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) _gdt_t;

// GDT entry struct taken from OSDev Wiki
typedef struct GDT_ENTRY {
	unsigned int limit_low 				: 16;
	unsigned int base_low				: 24;
	unsigned int accessed				: 1;
	unsigned int read_write 			: 1;
	unsigned int conform_expand_down	: 1;
	unsigned int code					: 1;
	unsigned int code_data_segment		: 1;
	unsigned int DPL					: 2;
	unsigned int present				: 1;
	unsigned int limit_high				: 4;
	unsigned int available				: 1;
	unsigned int long_mode				: 1;
	unsigned int big					: 1;
	unsigned int gran					: 1;
	unsigned int base_high				: 8;
} __attribute__((packed)) _gdt_entry_t;

typedef struct INTERRUPT_DESCRIPTOR_TABLE {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) _interrupt_descriptor_table_t;

typedef struct ISR_ENTRY {
	uint16_t offset_low;
	uint16_t selector;
	uint8_t  zero;
	uint8_t  type_attribute;
	uint16_t offset_high;
} __attribute__((packed)) _isr_entry_t;

// struct taken from OSDev Wiki
typedef struct TSS_ENTRY {
	uint32_t prev_tss; // The previous TSS - with hardware task switching these form a kind of backward linked list.
	uint32_t esp0;     // The stack pointer to load when changing to kernel mode.
	uint32_t ss0;      // The stack segment to load when changing to kernel mode.
	// Everything below here is unused.
	uint32_t esp1; // esp and ss 1 and 2 would be used when switching to rings 1 or 2.
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint16_t trap;
	uint16_t iomap_base;
} __attribute__((packed)) _tss_entry_t;

static _gdt_t global_descriptor_table;
static _gdt_entry_t descriptors[GDT_MAX];
static _interrupt_descriptor_table_t idt_entry;
static _isr_entry_t interrupts[MAX_INTERRUPTS];
static bool using_apic = false;

// this TSS will be used to get back to kernel land from user mode
_tss_entry_t kernel_tss;

/** default interrupt handler. set for all interrupts until initialization is complete */
extern void __attribute__((cdecl))i386_defaulthandler();

/** processor faults / traps / exceptions. the order in which they are listed is the order of the interrupt vector assigned */
extern void __attribute__((cdecl))i386_divbyzero();
extern void __attribute__((cdecl))i386_debugtrap();
extern void __attribute__((cdecl))i386_nmi();
extern void __attribute__((cdecl))i386_breakpoint();
extern void __attribute__((cdecl))i386_overflow();
extern void __attribute__((cdecl))i386_bounderror();
extern void __attribute__((cdecl))i386_badopcode();
extern void __attribute__((cdecl))i386_deviceerror();
extern void __attribute__((cdecl))i386_doublefault();
extern void __attribute__((cdecl))i386_invalidtss();
extern void __attribute__((cdecl))i386_segnotpresent();
extern void __attribute__((cdecl))i386_stacksegfault();
extern void __attribute__((cdecl))i386_genprotectfault();
extern void __attribute__((cdecl))i386_pagefault();
extern void __attribute__((cdecl))i386_fpuexception();
extern void __attribute__((cdecl))i386_aligncheck();
extern void __attribute__((cdecl))i386_machinecheck();
extern void __attribute__((cdecl))i386_simdexception();
extern void __attribute__((cdecl))i386_virtualizeexception();
extern void __attribute__((cdecl))i386_securityexception();

void __attribute__((naked)) syscall(void);

static inline void stop_interrupts() {
	asm volatile("cli");
}

static inline void start_interrupts() {
	asm volatile("sti");
}

static inline void load_gdtr() {
	asm volatile("lgdt (%0);" :: "m"(global_descriptor_table));
}

static inline void load_idtr() {
	asm volatile("lidt (%0);" :: "m"(idt_entry));
}

static inline void flush_tss() {
	__asm__(
		"movw $0x28,%ax\n\t"
		"ltr %ax\n\t"
	);
}

/** internal function declarations */
static void add_ring0_gdt_entries();
static void add_ring3_gdt_entries();
static void add_tss_gdt_entry();

static int install_handler(uint32_t index, uint8_t flags, uint16_t selector, uint32_t routine);
static void install_cpu_exceptions();
static void install_syscall();
static bool has_feature(uint32_t feature);

/**
 * Initialize the CPU. For now, I trust the GDT I set up in bootstub.asm
 * TODO: add code to work with the GDT / LDT
 */
int cpu_driver_init() {
	// interrupts are already off, but just in case...
	stop_interrupts();

	// set up a new global descriptor table (null out the descriptors)
	memset(&descriptors[0],0,sizeof(_gdt_entry_t)*GDT_MAX);
	add_ring0_gdt_entries();
	add_ring3_gdt_entries();
	add_tss_gdt_entry();

	global_descriptor_table.base = (uint32_t) &descriptors[0];
	global_descriptor_table.limit = (sizeof(_gdt_entry_t)*GDT_MAX)-1;

	load_gdtr();
	flush_tss();		// this has to happen *after* gdt setup. derp.
	
	//BOCHS_MAGIC_BREAKPOINT();

	// zero out the interrupt vector table
	memset(&interrupts[0],0,sizeof(_isr_entry_t)*MAX_INTERRUPTS);
	for (uint32_t i = 0; i < MAX_INTERRUPTS; i++) {
		install_handler(i,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_defaulthandler);
	}

	idt_entry.base = (uint32_t) &interrupts[0];
	idt_entry.limit = (sizeof(_isr_entry_t)*MAX_INTERRUPTS)-1;

	install_cpu_exceptions();
	install_syscall();
	load_idtr();

	// TODO: add APIC detection routine here and use 8259a PIC as fallback
	// going to disable the PICs for now so we aren't firing odd interrupts
	pic_8259a_initialize(DEVICE_INTERRUPT_BASE,DEVICE_INTERRUPT_BASE+8);
	pic_8259a_disable();

	start_interrupts();
	return 0;
}

/**
 * Tell the CPU to listen for interrupt requests from the given IRQ
 * Execute the handler at fn_address when the IRQ is raised
 */
void cpu_install_device(uint32_t irq, void *fn_address) {
	uint32_t interrupt_idx = irq + DEVICE_INTERRUPT_BASE;
	uint32_t address = (uint32_t) fn_address;
	
	// need to pause interrupts while making IDT changes
	stop_interrupts();
	install_handler(interrupt_idx,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,address);
	start_interrupts();
	
	if (!using_apic) {
		pic_8259a_unmask_irq((uint8_t) irq);
	} else {
		// TODO: add APIC routine here
	}
}

/** internal functions */

/**
 * install the given interrupt handler at the given index in the IDT
 */
static int install_handler(uint32_t index, uint8_t flags, uint16_t selector, uint32_t routine) {

	if (index >= MAX_INTERRUPTS) return -1;

	interrupts[index].offset_low = (uint16_t)(routine & 0xffff);
	interrupts[index].offset_high = (uint16_t)(routine >> 16);
	interrupts[index].selector = selector;
	interrupts[index].type_attribute = flags;
	return 0;
}

/**
 * install the CPU default exception handlers. This is only called once,
 * so it could be inline'd?
 */
static void install_cpu_exceptions() {
	// install the system exception handlers
	install_handler(0,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_divbyzero);
	install_handler(1,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_debugtrap);
	install_handler(2,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_nmi);
	install_handler(3,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_breakpoint);
	install_handler(4,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_overflow);
	install_handler(5,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_bounderror);
	install_handler(6,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_badopcode);
	install_handler(7,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_deviceerror);
	install_handler(8,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_doublefault);
	install_handler(10,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_invalidtss);
	install_handler(11,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_segnotpresent);
	install_handler(12,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_stacksegfault);
	install_handler(13,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_genprotectfault);
	install_handler(14,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_pagefault);
	install_handler(16,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_fpuexception);
	install_handler(17,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_aligncheck);
	install_handler(18,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_machinecheck);
	install_handler(19,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_simdexception);
	install_handler(20,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_virtualizeexception);
	install_handler(30,INTERRUPT32_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&i386_securityexception);
}

static void install_syscall() {
	extern void _syscall();
	install_handler(0x80,SYSCALL_INTERRUPT_TYPE,SYSTEM_CODE_SELECTOR,(uint32_t)&syscall);
}

/**
 * Check EDX for specific CPUID feature
 */
static bool has_feature(uint32_t feature) {
	if (__get_cpuid_max(CPUID_BASIC,NULL) > 0) {
		unsigned int reg_eax, reg_ebx, reg_ecx, reg_edx;
		__get_cpuid(CPUID_FEATURES, &reg_eax, &reg_ebx, &reg_ecx, &reg_edx);
		if (reg_edx & feature) {
			return true;
		}
	}
	return false;
}

static void add_ring0_gdt_entries() {
	_gdt_entry_t *ring0_code = &descriptors[1];
	_gdt_entry_t *ring0_data = &descriptors[2];

	ring0_code->limit_low 			= 0xffff;
	ring0_code->base_low  			= 0;
	ring0_code->accessed  			= 0;
	ring0_code->read_write 			= 1;
	ring0_code->conform_expand_down	= 0;
	ring0_code->code				= 1;
	ring0_code->code_data_segment 	= 1;
	ring0_code->DPL					= 0;
	ring0_code->present				= 1;
	ring0_code->limit_high			= 0xf;
	ring0_code->available			= 0;
	ring0_code->long_mode			= 0;
	ring0_code->big 				= 1;
	ring0_code->gran				= 1;
	ring0_code->base_high			= 0;

	*ring0_data = *ring0_code;
	ring0_data->code = 0;
}

static void add_ring3_gdt_entries() {
	_gdt_entry_t *ring3_code = &descriptors[3];
	_gdt_entry_t *ring3_data = &descriptors[4];

	ring3_code->limit_low 			= 0xffff;
	ring3_code->base_low  			= 0;
	ring3_code->accessed  			= 0;
	ring3_code->read_write 			= 1;
	ring3_code->conform_expand_down = 0;
	ring3_code->code				= 1;
	ring3_code->code_data_segment 	= 1;
	ring3_code->DPL					= 3;
	ring3_code->present				= 1;
	ring3_code->limit_high			= 0xf;
	ring3_code->available			= 0;
	ring3_code->long_mode			= 0;
	ring3_code->big 				= 1;
	ring3_code->gran				= 1;
	ring3_code->base_high			= 0;

	*ring3_data = *ring3_code;
	ring3_data->code = 0;
}

static void add_tss_gdt_entry() {
	_gdt_entry_t *tss_entry = &descriptors[5];

	uint32_t tss_base = (uint32_t) (&kernel_tss);
	printf("TSS Base: 0x%x\n", tss_base);

	uint32_t tss_limit = sizeof(_tss_entry_t);

	// add a descriptor for the TSS into the GDT
	tss_entry->limit_low			= tss_limit;
	tss_entry->base_low				= tss_base;
	tss_entry->accessed				= 1;		// this flag means something different for system entries. 1 means this is a TSS
	tss_entry->read_write			= 0;		// this flag represents whether or not the TSS is "busy"
	tss_entry->conform_expand_down 	= 0;		// always 0 for TSS
	tss_entry->code					= 1;
	tss_entry->code_data_segment 	= 0;
	tss_entry->DPL					= 3;
	tss_entry->present				= 1;
	tss_entry->limit_high			= (tss_limit & (0xf << 16)) >> 16;
	tss_entry->available			= 0;
	tss_entry->long_mode			= 0;
	tss_entry->big					= 0;
	tss_entry->gran					= 0;
	tss_entry->base_high			= (tss_base & (0xff << 24)) >> 24;

	//printf("tss base: 0x%x, tss limit: 0x%x\n", (uint32_t)((tss_entry->base_high << 24) | tss_entry->base_low),(uint32_t)(tss_entry->limit_high | tss_entry->limit_low));
	memset(&kernel_tss,0,sizeof(_tss_entry_t));
	kernel_tss.ss0 = 0x10;
	kernel_tss.esp0 = 0;
	//kernel_tss.iomap_base = sizeof(_tss_entry_t);
}

void set_kernel_stack(uint32_t esp) {
	kernel_tss.esp0 = esp;
}

#define MAX_SYSCALLS 1
void *syscalls[] = {
	puts
};

__attribute__((naked)) void syscall(void) {
	static int idx = 0;
	__asm__(
		"movl %%eax,%0"
		: "=m"(idx)
	);

	if (idx >= MAX_SYSCALLS)
	__asm__("iret");

	void *fn = syscalls[idx];

	__asm__(
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