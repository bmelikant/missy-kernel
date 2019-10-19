#ifndef _KERNEL_EARLY_UTILS_H
#define _KERNEL_EARLY_UTILS_H

#include <stddef.h>
#include <stdint.h>

#define MAXCHARS 65

extern void early_panic();

void *ki_memset(void *dst, int c, size_t len);
void *ki_memcpy(void *dst, const void *src, size_t count);
size_t ki_strlen(const char *str);
void ki_make_ulong_string(char *buffer, unsigned long number, int base);

inline uint8_t inportb_low(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1,%0" : "=a"(ret) : "Nd"(port));
	return ret;
}

inline void outportb_low(uint16_t port, uint8_t data) {
	asm volatile ("outb %0,%1" : : "a"(data), "Nd"(port));
}

#endif