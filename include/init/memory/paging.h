#ifndef _KERNEL_EARLY_PAGING_H
#define _KERNEL_EARLY_PAGING_H

#define ERROR_PAGING_ENOMEM 0x0001
#define KERNEL_PHYSICAL_TO_VIRTUAL(x) (x+0xc0000000)

int ki_setup_paging();
void *ki_get_kernel_end_virtual();
void *ki_get_kernel_stack_top();

#endif