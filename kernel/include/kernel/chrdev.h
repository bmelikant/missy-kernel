#ifndef _KERNEL_DEVICE_CHRDEV_H
#define _KERNEL_DEVICE_CHRDEV_H

#include <stdint.h>
#include <stddef.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct CHARACTER_DEVICE {
	int _descriptor;
	unsigned int major_revision;
	unsigned int minor_revision;
	bool is_open;
	int (*readchar)(void);
	int (*read)(char *buffer, size_t len);
	int (*write)(const char *buffer, size_t len);
	int (*open)(void);
	int (*close)(void);
} _chrdev_t;

int		register_device(_chrdev_t *device_descriptor);
int		read_device_char(int descriptor);
int 	read_device(int descriptor, char *buffer, size_t length);
int 	write_device(int descriptor, const char *buffer, size_t length);
int		open_device();
int		close_device();

#endif