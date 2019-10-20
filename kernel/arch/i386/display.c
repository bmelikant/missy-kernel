#include <kernel/display.h>
#include <kernel/ports.h>

#include <stdint.h>
#include <stddef.h>

#ifndef __build_i386
#error "Cannot build i386 driver display.c for non-i386 systems"
#endif

#define VIDEO_MEMORY_START 	0xb8000
#define VIDEO_COLUMNS 		80
#define VIDEO_ROWS 			25
#define COLOR_ATTRIBUTE 	0x0F

#define BLINKY_CURSOR 		0x5F
#define CLEAR_SCREEN_CHAR	0x20

#define TAB_CHAR_COUNT		4

static unsigned short x_loc = 0;
static unsigned short y_loc = 0;
static display_color_t color_attrib = 0x07;

/** internal functions */
static void disable_hardware_cursor();
static void update_blinky_cursor();
static void scroll_display();
static void increment_draw_location();
static void enable_blinking_text();
static void disable_blinking_text();
static uint16_t *get_write_location();
static void backspace(uint16_t *);
static void tabulate(uint16_t *);

void display_init() {
	// just going to disable the hardware cursor and implement our own
	disable_hardware_cursor();
	display_clear();
	enable_blinking_text();
}

void display_write(unsigned int c) {
	uint16_t *write_loc = get_write_location();
	if (c == '\b') {
		backspace(write_loc);
	} else if (c == '\t') {
		tabulate(write_loc);
	} else if (c == '\n') {
		*write_loc=(uint16_t)(color_attrib<<8)|CLEAR_SCREEN_CHAR;		// clear out the blinky cursor
		x_loc=0;
		y_loc++;
		update_blinky_cursor();
	} else {
		*write_loc = (uint16_t)((color_attrib << 8) | (uint8_t)(c));
		increment_draw_location();
	}
}

display_color_t display_change_color(display_color_t new_color) {
	display_color_t old_color = color_attrib;
	color_attrib = new_color;
	return old_color;
}

void backspace(uint16_t *write_loc) {
	*(--write_loc) = (uint16_t)(color_attrib<<8)|CLEAR_SCREEN_CHAR;
	if (x_loc == 0) {
		if (y_loc > 0) {
			y_loc--;
			x_loc = VIDEO_COLUMNS-1;
		}
	} else {
		x_loc--;
	}
}

void tabulate(uint16_t *write_loc) {
	for (size_t i = 0; i < TAB_CHAR_COUNT; i++) {
		*write_loc++ = (uint16_t)(color_attrib<<8)|CLEAR_SCREEN_CHAR;
		increment_draw_location();
	}
}

void increment_draw_location() {
	x_loc++;
	if (x_loc >= VIDEO_COLUMNS) {
		x_loc=0;
		y_loc++;
	}

	if (y_loc >= VIDEO_ROWS) {
		scroll_display();
	}

	update_blinky_cursor();
}


void disable_hardware_cursor() {
	outportb(0x3d4,0x0a);
	outportb(0x3d5,0x20);
}

void update_blinky_cursor() {
	// we are just going to leave the current x and y values intact
	// so the next write clears the cursor away
	uint16_t *cursor_loc = get_write_location();
	*cursor_loc = (uint16_t)(color_attrib<<8)|BLINKY_CURSOR;
}

void display_clear() {
	uint16_t *vmem = (uint16_t *) VIDEO_MEMORY_START;
	for (size_t i = 0; i < VIDEO_COLUMNS*VIDEO_ROWS; i++) {
		*(vmem+i) = (uint16_t)(color_attrib<<8)|CLEAR_SCREEN_CHAR;
	}
	x_loc = y_loc = 0;
	update_blinky_cursor();
}

void scroll_display() {
	uint16_t *dest = (uint16_t *) VIDEO_MEMORY_START;
	int copy_sz = VIDEO_COLUMNS*(VIDEO_ROWS-1);

	// copy all the characters up
	for (int i = 0; i < copy_sz; i++) {
		dest[i] = dest[i+VIDEO_COLUMNS];
	}

	// blank out last row
	for (int i = copy_sz; i < copy_sz+VIDEO_COLUMNS; i++) {
		dest[i] = (color_attrib << 8) | 0x20;
	}

	y_loc = VIDEO_ROWS-1;
}

uint16_t *get_write_location() {
	return (uint16_t *)(VIDEO_MEMORY_START + (y_loc*VIDEO_COLUMNS*2)+(x_loc*2));
}

void enable_blinking_text() {
	inportb(0x3da);		// discard this read. just to place the controller in the correct state
	uint8_t preserve = inportb(0x3c0);	// we need to preserve the value of this register for later

	outportb(0x3c0,0x10);					// read register index 10
	uint8_t attributes = inportb(0x3c1);	// for the text attribute bits
	attributes |= 0x08;						// toggle the blinky bit on
	outportb(0x3c0,attributes);				// and write back the bits
	outportb(0x3c0,preserve);				// and the preserved value
}

void disable_blinking_text() {
	inportb(0x3da);		// discard this read. just to place the controller in the correct state
	uint8_t preserve = inportb(0x3c0);	// we need to preserve the value of this register for later

	outportb(0x3c0,0x10);					// read register index 10
	uint8_t attributes = inportb(0x3c1);	// for the text attribute bits
	attributes &= ~0x08;						// toggle the blinky bit on
	outportb(0x3c0,attributes);				// and write back the bits
	outportb(0x3c0,preserve);				// and the preserved value
}