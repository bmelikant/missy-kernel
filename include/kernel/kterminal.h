#ifndef _KERNEL_TERMINAL_H
#define _KERNEL_TERMINAL_H

#include <stddef.h>


typedef struct KTERMINAL_PARAMETERS {
	unsigned int bytes_per_page;
} kterminal_params_t;

typedef unsigned int (*kterminal_read_callback)(size_t);
typedef void		 (*kterminal_write_callback)(unsigned int);
typedef kterminal_params_t (*kterminal_get_params_callback)();

typedef struct KTERMINAL_IO {
	kterminal_read_callback readchar;
	kterminal_write_callback writechar;
	kterminal_get_params_callback get_parameters;
} _kterminal_io_t;

#endif // _KERNEL_TERMINAL_H