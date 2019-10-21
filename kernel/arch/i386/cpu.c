#include <kernel/cpu.h>

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#ifndef __build_i386
#error "Cannot build cpu.c driver for non-i386 systems"
#endif

#define MAX_INTERRUPTS 256

#define INTERRUPT_TYPE_PRESENT		0x80
#define INTERRUPT_TYPE_SYSTEM		0x00
#define INTERRUPT_TYPE_USER			0x60
#define INTERRUPT_TYPE_INTERRUPT	0x0E

typedef void(*isr_handler)();

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

_interrupt_descriptor_table_t idt_entry;
_isr_entry_t interrupts[MAX_INTERRUPTS];

extern void __attribute__((cdecl))default_interrupt_vector();

static inline void stop_interrupts() {
	asm volatile("cli");
}

static inline void start_interrupts() {
	asm volatile("sti");
}

static inline void load_idtr() {
	asm volatile("lidt (%0);" :: "m"(idt_entry));
}

static void install_handler(uint32_t index, uint8_t flags, uint16_t selector, uint32_t routine) {
	if (index > MAX_INTERRUPTS) return;

	interrupts[index].offset_low = (uint16_t)(routine & 0xffff);
	interrupts[index].offset_high = (uint16_t)(routine >> 16);
	interrupts[index].selector = selector;
	interrupts[index].type_attribute = flags;
}

/**
 * Initialize the CPU. For now, I trust the GDT I set up in bootstub.asm
 */
int cpu_driver_init() {
	// interrupts are already off, but just in case...
	stop_interrupts();

	// zero out the interrupt vector table
	memset(&interrupts[0],0,sizeof(_isr_entry_t)*MAX_INTERRUPTS);
	for (uint32_t i = 0; i < MAX_INTERRUPTS; i++) {
		install_handler(i,INTERRUPT_TYPE_INTERRUPT|INTERRUPT_TYPE_PRESENT,0x08,(uint32_t)&default_interrupt_vector);
	}

	idt_entry.base = (uint32_t) &interrupts[0];
	idt_entry.limit = (sizeof(_isr_entry_t)*MAX_INTERRUPTS)-1;

	load_idtr();
	start_interrupts();
	return 0;
}