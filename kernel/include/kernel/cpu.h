#ifndef _KERNEL_DRIVER_I386_CPU_H
#define _KERNEL_DRIVER_I386_CPU_H

#include <stdint.h>

int cpu_driver_init();
void cpu_install_device(uint32_t irq_index,void *fn_address);

#endif