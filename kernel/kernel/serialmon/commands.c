/**
 * This file houses the commands for the built-in boot time
 * serial terminal. 
 */

#include <stdio.h>

typedef struct HASH_MAP _map_t;
typedef void(*_callback_p)(int, char **);

extern void store_item(_map_t *map, const char *key, _callback_p value);
extern void serial_write_string(int sd, char *str);

int sd_descriptor = 0;

int help(int, char **);

int register_commands(_map_t *map, int sd) {
	sd_descriptor = sd;
	store_item(map, "help", help);
}

/**
 * help() command: print the help message to the serial terminal
 */
int help(int argc, char** argv) {
	serial_write_string(sd_descriptor, "hello from the help command!\n");
}