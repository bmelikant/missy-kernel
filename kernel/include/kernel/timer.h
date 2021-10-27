#ifndef _KERNEL_DRIVER_I386_TIMER_H
#define _KERNEL_DRIVER_I386_TIMER_H 1

typedef int (*timer_callback_t)(void);

void pit_8254_initialize(void);
void pit_8254_start(unsigned int freq);
unsigned int pit_8254_get_ticks(void);

int timer_register_callback(timer_callback_t callback);

#endif // _KERNEL_DRIVER_I386_TIMER_H