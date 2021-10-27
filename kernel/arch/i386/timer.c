/**
 * i386 Programmable Interval Timer (PIT) driver
 */

#include <kernel/timer.h>
#include <kernel/cpu.h>
#include <kernel/ports.h>

#include <stdlib.h>
#include <string.h>

#define PIT_CHANNEL0_DATA_PORT 	0x40
#define PIT_CHANNEL1_DATA_PORT 	0x41
#define PIT_CHANNEL2_DATA_PORT 	0x42
#define PIT_COMMAND_PORT		0x43

#define PIT_CMD_CHANNEL_0		0x00
#define PIT_CMD_CHANNEL_1		0x40
#define PIT_CMD_CHANNEL_2		0x80
#define PIT_READBACK_COMMAND	0xc0

#define PIT_CMD_LATCH_COUNT		0x00
#define PIT_CMD_LOBYTE_ONLY		0x10
#define PIT_CMD_HIBYTE_ONLY		0x20
#define PIT_CMD_LOBYTE_FIRST	0x30

#define PIT_CMD_IOTC		0x00
#define PIT_CMD_ONESHOT 	0x02
#define PIT_CMD_RATEGEN		0x04
#define PIT_CMD_SQUAREWAVE	0x06
#define PIT_CMD_SW_STROBE	0x08
#define PIT_CMD_HW_STROBE	0x0a

#define PIT_CMD_BINARY_MODE		0
#define PIT_CMD_BCD_MODE		1

#define PIT_CLOCK_FREQ 1193182

#define PIT_DEFAULT_STARTUP_COMMAND (PIT_CMD_CHANNEL_0 | PIT_CMD_LOBYTE_FIRST | \
	PIT_CMD_SQUAREWAVE | PIT_CMD_BINARY_MODE)

// tick counter / interrupt vector
unsigned int pit_tick_count = 0;
extern void __attribute__((cdecl)) timer_irq(void);

#define TIMER_MAX_CALLBACKS 10

static timer_callback_t callbacks[TIMER_MAX_CALLBACKS];
static int callback_idx = 0;

// void pit_8254_initialize (): Initialize the programmable interval timer
// 8253-compatible chipset
void pit_8254_initialize(void) {
	// install a device to IRQ0
	cpu_install_device (0,&timer_irq);
	pit_8254_start(100);
}

// void pit_8254_start (): Change the timer frequency and start a new timer
// at freq Hz. timer trips IRQ0 when fired
void pit_8254_start(unsigned int freq) {

	// calculate the frequency value
	unsigned short reload_value = PIT_CLOCK_FREQ / freq;

	// write the command, then the values to the system
	outportb (PIT_COMMAND_PORT, PIT_DEFAULT_STARTUP_COMMAND);
	outportb (PIT_CHANNEL0_DATA_PORT, (unsigned char)(reload_value & 0xff00));
	outportb (PIT_CHANNEL0_DATA_PORT, (unsigned char)(reload_value >> 8));
}

int timer_register_callback(timer_callback_t callback) {
	if (callback_idx < TIMER_MAX_CALLBACKS - 1) {
		callbacks[callback_idx++] = callback;
		return 0;
	}

	return -1;
}

unsigned int pit_8254_get_ticks(void) {
	return pit_tick_count;
}

void pit_8254_process_callbacks(void) {
	for (int i = 0; i < callback_idx; i++) {
		callbacks[i]();
	}
}