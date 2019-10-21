#ifndef _KERNEL_DRIVER_I386_TIMER_H
#define _KERNEL_DRIVER_I386_TIMER_H 1

void pit_8254_initialize(void);
void pit_8254_start(unsigned int freq);
unsigned int pit_8254_get_ticks(void);

#endif // _KERNEL_DRIVER_I386_TIMER_H