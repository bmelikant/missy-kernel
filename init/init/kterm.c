#include <init/kterm.h>
#include <init/kutils.h>

#ifndef _cplusplus
#include <stdbool.h>
#endif

#define VIDEO_MEMORY_BASE 		0xB8000
#define VIDEO_MAX_COLS			80
#define VIDEO_MAX_ROWS			25
#define VIDEO_COLOR_ATTRIBUTE 	0x0F
#define TAB_SIZE 				4

#define TEXT_INFO_COLOR 	0x2F
#define TEXT_PANIC_COLOR 	0xCF
#define TEXT_WARNING_COLOR  0xEF
#define TEXT_LOG_COLOR 		VIDEO_COLOR_ATTRIBUTE

// private (internal) variables
static int _cur_x = 0;
static int _cur_y = 0;
static uint8_t color_attribute = VIDEO_COLOR_ATTRIBUTE;

int kernel_early_puts(const char *str) {
	char *s = (char *) str;
	
	while (*s) {
		ki_putc(*s++);
	}

	ki_putc('\n');
	ki_update_cursor();
	return (int)(s-str)+1;
}

int kernel_early_putstr(const char *str) {
	char *s = (char *) str;
	while (*s) {
		ki_putc(*s++);
	}
	ki_update_cursor();
	return (int)(s-str)+1;
}

int ki_putc(int c) {

	if (c == '\n') {
		_cur_x = 0;
		_cur_y++;
	} else if (c == '\t') {
		for (int i = 0; i < TAB_SIZE; i++) {
			ki_putc(0x20);
		}
	} else if (c == '\b') {
		_cur_x--;
		ki_putc(0x20);
		_cur_x--;
	} else {
		// grab a pointer to the location in video memory
		uint16_t *video_ptr = (uint16_t *) (VIDEO_MEMORY_BASE+(_cur_y*VIDEO_MAX_COLS*2)+(_cur_x*2));
		*video_ptr = (uint16_t) (color_attribute << 8) | c;
		_cur_x++;
	
		if (_cur_x >= VIDEO_MAX_COLS) {
			_cur_x = 0;
			_cur_y++;
		}
	}

	// at the end of this function, if _cur_y > max rows scroll the display
	if (_cur_y > VIDEO_MAX_ROWS-1) {
		kernel_early_scroll_display();
	}

	return c;
}

void kernel_early_scroll_display() {
	uint16_t *dest = (uint16_t *) VIDEO_MEMORY_BASE;
	int copy_sz = VIDEO_MAX_COLS*(VIDEO_MAX_ROWS-1);

	// copy all the characters up
	for (int i = 0; i < copy_sz; i++) {
		dest[i] = dest[i+VIDEO_MAX_COLS];
	}

	// blank out last row
	for (int i = copy_sz; i < copy_sz+VIDEO_MAX_COLS; i++) {
		dest[i] = (VIDEO_COLOR_ATTRIBUTE << 8) | 0x20;
	}

	_cur_y = VIDEO_MAX_ROWS-1;
}

void kernel_early_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outportb_low(0x3D4, 0x0A);
	outportb_low(0x3D5, inportb_low(0x3D5 & 0xC0) | cursor_start);
	outportb_low(0x3D4, 0x0B);
	outportb_low(0x3D5, inportb_low(0x3D5 & 0xE0) | cursor_end);
}

void ki_update_cursor() {
	uint16_t pos = _cur_y * VIDEO_MAX_COLS + _cur_x;
	outportb_low(0x3D4, 0x0F);
	outportb_low(0x3D5, (uint8_t) (pos & 0xff));
	outportb_low(0x3D4, 0x0E);
	outportb_low(0x3D5, (uint8_t) ((pos >> 8) & 0xff));
}

uint8_t kernel_early_set_color_attr(uint8_t color) {
	uint8_t current = color_attribute;
	color_attribute = color;
	return current;
}

int ki_printf(const char *str, ...) {
	// use va_start to get the list of items
	va_list args;
	va_start (args, str);

	while (*str) {

		// if the '%' character appears, it is a formatting request
		if (*str == '%') {
			str++;		// increment the string pointer to get the format item
			// is an integer print requested?
			if (*str == 'd' || *str == 'i') {
				unsigned int argument = va_arg(args, unsigned int);
				char num_str[MAXCHARS];

				ki_make_ulong_string(num_str, argument, 10);
				for (size_t i = 0; i < ki_strlen(num_str); i++) {
					ki_putc(num_str[i]);
				}
			} else if (*str == 'b') {
				unsigned long argument = va_arg(args, unsigned long);
				char num_str[MAXCHARS];

				ki_make_ulong_string(num_str, argument, 2);
				for (size_t i = 0; i < ki_strlen (num_str); i++) {
					ki_putc(num_str[i]);
				}
			
			} else if (*str == 'X' || *str == 'x') {
				unsigned long argument = va_arg (args, unsigned long);
				char num_str[MAXCHARS];

				ki_make_ulong_string(num_str, argument, 16);
				for (size_t i = 0; i < ki_strlen (num_str); i++) {
					ki_putc(num_str[i]);
				}

			} else if (*str == 's') {
				char *argument = va_arg(args, char*);
				while (*argument)
					ki_putc(*argument++);
			
			} else if (*str == 'c') {
				unsigned char argument = (unsigned char) va_arg(args, unsigned int);
				ki_putc(argument);
			} else {
				ki_putc(*str);
			}

			// increment the pointer
			str++;
		}

		else
			ki_putc(*str++);		// just print out the character
	}

	va_end(args);
	ki_update_cursor();
	return 0;
}