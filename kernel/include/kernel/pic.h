#ifndef _KERNEL_DRIVER_I386_PIC_H
#define _KERNEL_DRIVER_I386_PIC_H 1

#include <stdint.h>

int pic_8259a_initialize(uint8_t pic1_offset, uint8_t pic2_offset);
void pic_8259a_mask_irq (uint8_t irq);
void pic_8259a_unmask_irq (uint8_t irq);
void pic_8259a_enable(void);
void pic_8259a_disable(void);
void pic_8259a_send_eoi(uint8_t irq);
uint32_t pic_8259a_spurious_count();

#endif