#ifndef _KERNEL_EARLY_PAGING_H
#define _KERNEL_EARLY_PAGING_H

#define ERROR_PAGING_ENOMEM 0x0001

int ki_setup_paging();
void *ki_get_kernel_end_virtual();

#endif