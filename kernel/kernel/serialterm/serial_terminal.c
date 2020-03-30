#include <kernel/serial.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct HASH_MAP _map_t;
typedef int(*_callback_p)(int, char **);

_map_t *new_map(size_t initial_bucket_count);
void store_item(_map_t *map, const char *key, _callback_p value);
_callback_p get_item(_map_t *map, const char *key) ;

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
void serial_terminal(const char *prompt) {
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
}

/** boot-time terminal command caching functions. uses hashmap to store commands */
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

#define BUCKET_SIZE			10					// 10 items per bucket
#define INITIAL_BUCKETS		10					// initially there will be 10 buckets
#define MINIMUM_BUCKETS 	INITIAL_BUCKETS		// we can never have less buckets than the initial number of buckets

/** internal map structure */
typedef struct BUCKET_ITEM {
	const char *key;			// the key string
	_callback_p value;		//callback to command function
} _bucket_item_t;

// easier to hold a list of map items this way
typedef _bucket_item_t * _bucket_item_p;

/** internal map structure */
typedef struct ITEM_BUCKET {
	_bucket_item_p *bucket_items;	// the list of items stored in this bucket
	size_t items_count;				// the number of items currently stored in this bucket
} _item_bucket_t;

/** typedef struct HASH_MAP _map_t (include/map.h) */
struct HASH_MAP {
	_item_bucket_t *buckets;
	size_t bucket_count;
};


/** static function declarations */
static int store_to_bucket(_item_bucket_t *, const char *, _callback_p);	// store an item in a bucket
static int remove_from_bucket(_item_bucket_t *, const char *);				// delete an item from a bucket
static int initialize_bucket(_item_bucket_t *);								// allocate a bucket to hold items
static int free_bucket(_item_bucket_t *);									// releases an empty bucket
static _bucket_item_t *find_in_bucket(_item_bucket_t *, const char *);		// try to find the given key in the given bucket

static unsigned long hashsum(const char *, size_t);		// compute the hash sum for the given key

/** api functions */

/**
 * this function will create a new hash map using the specified map parameters
 * if the given parameters are below the minimums, the minimums will be used
 * returns: a pointer to the new map struct if successful; null otherwise
 */
_map_t *new_map(size_t initial_bucket_count) {
	// create the new map
	_map_t *new_map_ptr = (_map_t *) malloc(sizeof(_map_t));
	if (!new_map_ptr) return NULL;
	DEBUG("allocated map pointer\n");

	new_map_ptr->bucket_count = (initial_bucket_count >= MINIMUM_BUCKETS) ? initial_bucket_count : MINIMUM_BUCKETS;

	// allocate the list of bucket items and initialize them to zero
	new_map_ptr->buckets = (_item_bucket_t *) malloc(sizeof(_item_bucket_t)*new_map_ptr->bucket_count);
	if (!new_map_ptr->buckets) { free(new_map_ptr); return NULL; }
	DEBUG("allocated item bucket list\n");
	memset(new_map_ptr->buckets,0,sizeof(_item_bucket_t)*new_map_ptr->bucket_count);
	DEBUG("zeroed item bucket list storage\n");

	return new_map_ptr;
}

/**
 * add an element to the hash map
 */
void store_item(_map_t *map, const char *key, _callback_p value) {
	unsigned long hashvalue = hashsum(key, map->bucket_count);
	DEBUG("hash value of %s is %li\n", key, hashvalue);
	_item_bucket_t *item_bucket = map->buckets+hashvalue;
	int retval = store_to_bucket(item_bucket, key, value);
	DEBUG("%d\n", retval);
}

/**
 * get an element from the hash map
 */
_callback_p get_item(_map_t *map, const char *key) {
	unsigned long hashvalue = hashsum(key, map->bucket_count);
	DEBUG("hash value of %s is %li\n", key, hashvalue);
	_item_bucket_t *item_bucket = map->buckets+hashvalue;
	DEBUG("there are %li items stored in this bucket\n", item_bucket->items_count);
	_bucket_item_t *item = find_in_bucket(item_bucket, key);
	if (!item) { DEBUG("item not found in bucket\n"); return NULL; }

	DEBUG("found item in bucket; returning item value\n");
	return item->value;
}

/** static functions */

/**
 * store an item in the given bucket. the bucket should be initialized before use
 */
int store_to_bucket(_item_bucket_t *bucket, const char *key, _callback_p value) {
	// we can't use a bucket that doesn't exist
	if (!bucket) { return -1; }
	// if the bucket needs initialized, do it
	if (!bucket->bucket_items) { if (initialize_bucket(bucket) == -1) return -1; }
	// bucket is full -> TODO - handle adding more buckets to the map
	if (bucket->items_count == BUCKET_SIZE-1) { return -2; }

	// now, check if the item is already in the bucket. build a new item if not
	_bucket_item_t *bucket_item = find_in_bucket(bucket, key);
	if (!bucket_item) { 
		bucket_item = (_bucket_item_t *) malloc(sizeof(_bucket_item_t));
		if (!bucket_item) return -1;
		bucket_item->key = (const char *) malloc(strlen(key));
		if (!bucket_item->key) { free(bucket_item); return -1; }
		strncpy(bucket_item->key, key, strlen(key));
	}
	
	bucket_item->value = value;

	// store the item in the current bucket index
	DEBUG("storing item in bucket at index %li\n", bucket->items_count);
	bucket->bucket_items[bucket->items_count++] = bucket_item;
	return 0;
}

/**
 * initializes a bucket for use. once the bucket is created, items can be freely stored inside it.
 */
int initialize_bucket(_item_bucket_t *bucket) {
	// don't initialize a bucket that is invalid
	if (!bucket) { return -1; }

	bucket->items_count = 0;
	bucket->bucket_items = (_bucket_item_p *) malloc(sizeof(_bucket_item_p)*BUCKET_SIZE);
	if (!bucket->bucket_items) return -1;

	return 0;
}

/**
 * find an item stored in the given bucket by key and return a pointer to the item
 * if the item is not found in the bucket, a NULL pointer is returned
 */
_bucket_item_t *find_in_bucket(_item_bucket_t *bucket, const char *key) {
	for (size_t i = 0; i < bucket->items_count; i++) {
		_bucket_item_t *current_item = bucket->bucket_items[i];
		if (strlen(current_item->key) != strlen(key)) continue;
		if (strncmp(current_item->key, key, strlen(key)) == 0) return current_item;
	}
	return NULL;
}

/**
 * compute the hash sum of a given string
 */
unsigned long hashsum(const char *key, size_t elements) {
	unsigned long hash = 0;
	while (*key) {
		unsigned long highorder = (unsigned long) hash & 0xf8000000;
		hash <<= 5;
		hash ^= (highorder >> 27);
		hash ^= (unsigned long)(*key++);
	}

	DEBUG("current hash sum: %li\n", hash);
	return (unsigned long)(hash % elements);
}

/**
 * This function will allow a built-in command to be registered with this terminal
 * 
 */