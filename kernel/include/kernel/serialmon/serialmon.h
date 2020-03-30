#ifndef _KERNEL_SERIAL_MONITOR_H
#define _KERNEL_SERIAL_MONITOR_H

#include <stdio.h>

typedef struct HASH_MAP _map_t;
typedef int(*_callback_p)(int, char **);

/** serial monitor initialization */
int serial_monitor(const char *prompt);

/** hashmap functions */
_map_t *new_map(size_t);
void store_item(_map_t *map, const char *key, _callback_p fn);
_callback_p get_item(_map_t *map, const char *key);

#ifdef DEBUG_BUILD
#define WARN(...) 			\
do {						\
	printf("ERROR: "); 		\
	printf(__VA_ARGS__); 	\
} while (0)
#define DEBUG(...)			\
do {						\
	printf("DEBUG: ");		\
	printf(__VA_ARGS__);	\
} while (0)
#else
#define WARN(...)
#define DEBUG(...)
#endif

#endif