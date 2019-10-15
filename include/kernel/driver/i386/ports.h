#ifndef _KERNEL_PORT_IO_H
#define _KERNEL_PORT_IO_H

#include <stdint.h>

#ifndef __build_i386
#error "Cannot use i386-specific port I/O functions on non-i386 builds"
#endif

inline uint8_t inportb(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1,%0" : "=a"(ret) : "Nd"(port));
	return ret;
}

inline void outportb(uint16_t port, uint8_t data) {
	asm volatile ("outb %0,%1" : : "a"(data), "Nd"(port));
}

#endif