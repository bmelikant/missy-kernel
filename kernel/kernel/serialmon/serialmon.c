#include <kernel/serialmon/serialmon.h>
#include <kernel/serial.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int register_commands(_map_t *map, int sd);

void serial_write_string(int sd, char *str) {
	write_device(sd, str, strlen(str));
}

int serial_read_command(int sd, char *buffer, int max_length) {
	int c = 0;
	int count = 0;
	while ((c = read_device_char(sd)) != -1 && count < max_length) {
		if (c == '\r') {
			c = '\n';
			write_device(sd, &c, 1);
			break;
		} else if (c == '\b') {
			buffer[--count] = 0;
			int bkspc = '\b', spc = ' ';
			write_device(sd, &bkspc, 1);
			write_device(sd, &spc, 1);
			write_device(sd, &bkspc, 1);
		} else {
			buffer[count++] = c;
			write_device(sd, &c, 1);
		}
	}
	return count;
}

/**
 * This function will be removed following merge of this branch
 * This presents a simple terminal to the serial port
 */
int serial_monitor(const char *prompt) {
	int dd = serial_init(COM_PORT_1);
	printf("COM port device descriptor: %d\n", dd);

	// create the command hash map
	_map_t *command_map = new_map(10);
	register_commands(command_map, dd);

	write_device(dd, banner(), strlen(banner()));
	char *cmd_buffer = (char *) malloc(255);
	memset(cmd_buffer,0,255);

	for (;;) {
		serial_write_string(dd, prompt);
		int length = serial_read_command(dd, cmd_buffer, 255);
		printf("%s\n", cmd_buffer);
		_callback_p fn;

		if (strncmp(cmd_buffer, "stop", strlen(cmd_buffer)) == 0) {
			serial_write_string(dd, "STOP command was issued\n");
			break;
		} else if ((fn = get_item(command_map, cmd_buffer))) {
			fn(0, "");
		} else {
			serial_write_string(dd, "Unknown command: ");
			serial_write_string(dd, cmd_buffer);
			serial_write_string(dd, "\n");
		}

		// clear out the command buffer
		memset(cmd_buffer, 0, 255);
	}

	return 0;
}