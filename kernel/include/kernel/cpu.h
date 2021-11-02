#ifndef _KERNEL_DRIVER_I386_CPU_H
#define _KERNEL_DRIVER_I386_CPU_H

#include <stdint.h>
#include <stdlib.h>

int cpu_driver_init();
void cpu_install_device(uint32_t irq_index,void *fn_address);

//extern void __attribute__((naked)) enter_usermode();

static inline void enter_usermode() {
    __asm__(
        "movl %esp,%eax\n\t"
        "push %eax\n\t"
        "call set_kernel_stack\n\t"
        "addl $4,%esp\n\t"
        "movw $0x23,%ax\n\t"
        "mov %ax,%ds\n\t"
        "mov %ax,%es\n\t"
        "mov %ax,%fs\n\t"
        "mov %ax,%gs\n\t"
        "push $0x23\n\t"
        "push %esp\n\t"
        "pushf\n\t"
        "pop %eax\n\t"
        "orl $0x200,%eax\n\t"
        "push %eax\n\t"
        "push $0x1B\n\t"
        "lea userspace,%eax\n\t"
        "push %eax\n\t"
        "iretl\n\t"
        "userspace:\n\t"
        "addl $4,%esp"
    );
}

#endif