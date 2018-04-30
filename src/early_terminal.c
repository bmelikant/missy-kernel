#include <early_terminal.h>

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

const char 	lettermap [] = "0123456789abcdef";
char		buffer[32];
extern int 	errno;

static inline uint8_t inportb_low(uint16_t port) {
	uint8_t ret;
	asm volatile ("inb %1,%0" : "=a"(ret) : "Nd"(port));
	return ret;
}

static inline void outportb_low(uint16_t port, uint8_t data) {
	asm volatile ("outb %0,%1" : : "a"(data), "Nd"(port));
}

static size_t strlen(const char *str) {
	char *s = (char *) str;
	while (*s++);
	return (size_t) (s-str)-1;
}

static char * itoa_s(char *str, int num, int base) {
	int pos  = 0;
	int opos = 0;
	int top  = 0;

	// base must be < 16 and i cannot be zero
	if (num == 0 || base > 16) {

		str[0] = '0';
		str[1] = '\0';

		return str;
	}

	// now transform the number to a string
	while (num > 0) {

		buffer[pos] = lettermap[num % base];
		pos++;
		num /= base;
	}

	top = pos--;

	for (opos = 0; opos < top; pos--,opos++)
		str[opos] = buffer[pos];

	str[opos] = 0;
	return str;
}

int kernel_early_puts(const char *str) {
	char *s = (char *) str;
	
	while (*s) {
		kernel_early_putc(*s++);
	}

	kernel_early_putc('\n');
	kernel_early_update_cursor();
	return (int)(s-str)+1;
}

int kernel_early_putstr(const char *str) {
	char *s = (char *) str;
	while (*s) {
		kernel_early_putc(*s++);
	}
	kernel_early_update_cursor();
	return (int)(s-str)+1;
}

int kernel_early_putc(int c) {

	if (c == '\n') {
		_cur_x = 0;
		_cur_y++;
		
	} else if (c == '\t') {
		for (int i = 0; i < TAB_SIZE; i++) {
			kernel_early_putc(0x20);
		}

	} else if (c == '\b') {
		_cur_x--;
		kernel_early_putc(0x20);
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

int kernel_early_printf(const char *str, ...) {

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
				char num_str[20];

				itoa_s(num_str, argument, 10);

				for (size_t i = 0; i < strlen(num_str); i++)
					kernel_early_putc(num_str[i]);
			
			} else if (*str == 'b') {

				unsigned int argument = va_arg(args, unsigned int);
				char num_str[36];

				itoa_s(num_str, argument, 2);

				for (size_t i = 0; i < strlen (num_str); i++)
					kernel_early_putc(num_str[i]);
			
			} else if (*str == 'X' || *str == 'x') {

				unsigned int argument = va_arg (args, unsigned int);
				char num_str[20];

				itoa_s (num_str, argument, 16);

				for (size_t i = 0; i < strlen (num_str); i++)
					kernel_early_putc(num_str[i]);
			
			} else if (*str == 's') {

				char *argument = va_arg (args, char*);

				while (*argument)
					kernel_early_putc(*argument++);
			
			} else if (*str == 'c') {

				unsigned char argument = (unsigned char) va_arg (args, unsigned int);
				kernel_early_putc(argument);

			} else if (*str == 'P') {

				kernel_early_log("[PANIC]", TEXT_PANIC_COLOR);
			
			} else if (*str == 'I') {

				kernel_early_log("[INFO]", TEXT_INFO_COLOR);

			} else if (*str == 'W') {

				kernel_early_log("[WARNING]", TEXT_WARNING_COLOR);

			} else if (*str == 'L') {

				kernel_early_log("[KERNEL]", TEXT_LOG_COLOR);
			
			} else {
				kernel_early_putc(*str);
			}

			// increment the pointer
			str++;
		}

		else
			kernel_early_putc(*str++);		// just print out the character
	}

	kernel_early_update_cursor();
	return 0;
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

void kernel_early_log(const char *level, uint8_t color) {
	uint8_t current_color = kernel_early_set_color_attr(color);
	kernel_early_putstr(level);
	kernel_early_set_color_attr(current_color);
	kernel_early_putstr(": ");
}

void kernel_early_enable_cursor(uint8_t cursor_start, uint8_t cursor_end) {
	outportb_low(0x3D4, 0x0A);
	outportb_low(0x3D5, inportb_low(0x3D5 & 0xC0) | cursor_start);
	outportb_low(0x3D4, 0x0B);
	outportb_low(0x3D5, inportb_low(0x3D5 & 0xE0) | cursor_end);
}

void kernel_early_update_cursor() {
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