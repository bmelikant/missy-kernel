#ifndef _KERNEL_EARLY_UTILS_H
#define _KERNEL_EARLY_UTILS_H

#include <stddef.h>
#include <stdint.h>

extern void early_panic();

void *early_memset(void *dst, int c, size_t len);
size_t early_strlen(const char *str);
char *early_itoa_s(char *str, int num, int base);

inline uint8_t inportb_low(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1,%0" : "=a"(ret) : "Nd"(port));
	return ret;
}

inline void outportb_low(uint16_t port, uint8_t data) {
	asm volatile ("outb %0,%1" : : "a"(data), "Nd"(port));
}

#endif