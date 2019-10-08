#ifndef _KERNEL_EARLY_TERMINAL_H
#define _KERNEL_EARLY_TERMINAL_H

#include <stddef.h>
#include <stdint.h>
#include <stdarg.h>

int kernel_early_putc(int c);
int kernel_early_puts(const char *str);
int kernel_early_putstr(const char *str);
int kernel_early_printf(const char *str, ...);
void kernel_early_log(const char *level, uint8_t color);

void kernel_early_scroll_display();
void kernel_early_enable_cursor(uint8_t, uint8_t);
void kernel_early_update_cursor();
uint8_t kernel_early_set_color_attr(uint8_t color);

#endif