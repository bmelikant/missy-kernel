#ifndef _KERNEL_DRIVER_I386_DISPLAY_H
#define _KERNEL_DRIVER_I386_DISPLAY_H

#include <stdint.h>

typedef uint8_t display_color_t;

#define COLOR_FG_BLACK 			0x0
#define COLOR_FG_BLUE			0x1
#define COLOR_FG_GREEN			0x2
#define COLOR_FG_CYAN			0x3
#define COLOR_FG_RED			0x4
#define COLOR_FG_PURPLE			0x5
#define COLOR_FG_BROWN			0x6
#define COLOR_FG_GRAY			0x7
#define COLOR_FG_DARKGRAY		0x8
#define COLOR_FG_LIGHTBLUE		0x9
#define COLOR_FG_LIGHTGREEN		0xA
#define COLOR_FG_LIGHTCYAN		0xB
#define COLOR_FG_LIGHTRED		0xC
#define COLOR_FG_LIGHTPURPLE	0xD
#define COLOR_FG_YELLOW			0xE
#define COLOR_FG_WHITE			0xF

#define COLOR_BG_BLACK 	COLOR_FG_BLACK
#define COLOR_BG_BLUE	COLOR_FG_BLUE
#define COLOR_BG_GREEN	COLOR_FG_GREEN
#define COLOR_BG_CYAN	COLOR_FG_CYAN
#define COLOR_BG_RED	COLOR_FG_RED
#define COLOR_BG_PURPLE	COLOR_FG_PURPLE
#define COLOR_BG_BROWN	COLOR_FG_BROWN
#define COLOR_BG_GRAY	COLOR_FG_GRAY

#define COLOR_BG_BLINK_BLACK	(COLOR_BG_BLACK & 0x8)
#define COLOR_BG_BLINK_BLUE		(COLOR_BG_BLUE & 0x8)
#define COLOR_BG_BLINK_GREEN	(COLOR_BG_GREEN & 0x8)
#define COLOR_BG_BLINK_CYAN		(COLOR_BG_CYAN & 0x8)
#define COLOR_BG_BLINK_RED		(COLOR_BG_RED & 0x8)
#define COLOR_BG_BLINK_PURPLE	(COLOR_BG_PURPLE & 0x8)
#define COLOR_BG_BLINK_BROWN	(COLOR_BG_BROWN & 0x8)
#define COLOR_BG_BLINK_GRAY		(COLOR_BG_GRAY & 0x8)

#define display_make_color(fg,bg)	(display_color_t)((bg << 4)|fg)

void display_init();
void display_write(unsigned int i);
display_color_t display_change_color(display_color_t);
void display_clear();

#endif // _KERNEL_DRIVER_I386_DISPLAY_H