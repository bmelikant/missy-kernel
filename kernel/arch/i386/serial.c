#include <kernel/serial.h>
#include <kernel/chrdev.h>
#include <kernel/ports.h>
#include <kernel/cpu.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define SERIAL_MAJOR_REVISION 	0x0
#define IRQ_INDEX				0x04

#define DATA_REGISTER				0
#define INTERRUPT_ENABLE_REGISTER	1
#define DIVISOR_VALUE_LSB			0
#define DIVISOR_VALUE_MSB			1
#define II_FIFO_CONTROL_REGISTER	2
#define LINE_CONTROL_REGISTER		3
#define MODEM_CONTROL_REGISTER		4
#define LINE_STATUS_REGISTER		5
#define MODEM_STATUS_REGISTER		6
#define SCRATCH_REGISTER			7

#define ENABLE_DLAB		0x80
#define EIGHT_N_ONE		0x03
#define MAX_QUEUE		0x100		// 256 chars max

#define SERIAL_BACKSPACE 			127
#define SERIAL_END_OF_TEXT			3
#define SERIAL_END_OF_TRANSMISSION	4

static _chrdev_t *device_descriptor;
char buffer[MAX_QUEUE];
size_t index = 0;

uint16_t _port = 0;

extern void __attribute((cdecl)) _serial_irq(void);

static int initialize_serial_device(uint16_t);
static void send_character(char c);

/** descriptor functions */
int serial_read_char(void);
int serial_read_chars(char *buffer, size_t length);
int serial_write_chars(const char *buffer, size_t length);
int serial_open(void);
int serial_close(void);

int serial_init(int port) {
	initialize_serial_device((uint16_t) port);

	device_descriptor = (_chrdev_t *) malloc(sizeof(_chrdev_t));
	if (!device_descriptor) {
		printf("Error allocating memory for serial device\n");
		return -1;
	}

	// the _descriptor field should go through as zero; it will be filled in by devmngr
	device_descriptor->_descriptor = 0;

	// set up the device descriptor
	device_descriptor->readchar = serial_read_char;
	device_descriptor->read = serial_read_chars;
	device_descriptor->write = serial_write_chars;
	device_descriptor->open = serial_open;
	device_descriptor->close = serial_close;
	device_descriptor->major_revision = 0x0;

	// register this character device
	return register_device(device_descriptor);
}

int serial_read_chars(char *buffer, size_t length) {
	return 0;
}

int serial_read_char(void) {
	while ((inportb(_port+LINE_STATUS_REGISTER) & 1) == 0) ;

	char c = inportb(_port);
	
	// for some reason, backspace comes in as 127. translate to backspace
	if (c == SERIAL_BACKSPACE) {
		c = '\b';
	} else if (c == SERIAL_END_OF_TEXT) {
		c = -1;
	}
	
	return c;
}

int serial_write_chars(const char *buffer, size_t length) {
	for (size_t i = 0; i < length; i++) {
		send_character(buffer[i]);
	}
	return (int) length;
}

int serial_open(void) {
	return 0;
}

int serial_close(void) {
	return 0;
}

static int initialize_serial_device(uint16_t port) {
	cpu_install_device(IRQ_INDEX,&_serial_irq);

	outportb(port+INTERRUPT_ENABLE_REGISTER,0);			// disable serial interrupts
	outportb(port+LINE_CONTROL_REGISTER,ENABLE_DLAB);	// enable write of clock divisor
	outportb(port+DIVISOR_VALUE_LSB,0x03);
	outportb(port+DIVISOR_VALUE_MSB,0x00);				// 38400 baud
	outportb(port+LINE_CONTROL_REGISTER,EIGHT_N_ONE);	// eight bits, no parity, one stop
	outportb(port+II_FIFO_CONTROL_REGISTER,0xC7);		// magic value until I read more specs
	outportb(port+MODEM_CONTROL_REGISTER,0x0B);			// another magic value
	//outportb(port+INTERRUPT_ENABLE_REGISTER,1);			// enable the interrupt functionality?
	_port = port;
}

void _read_serial_input() {
	
}

void send_character(char c) {
	while ((inportb(_port+LINE_STATUS_REGISTER) & 0x20) == 0) ;
	outportb(_port,c);
}