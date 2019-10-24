#ifndef _KERNEL_ARCH_I386_PAGEMNGR_H
#define _KERNEL_ARCH_I386_PAGEMNGR_H 1

#define PAGEMNGR_PAGE_SIZE 	4096
#define PAGEMNGR_PAGE_ALIGN_UP(x)	((x &~ 0xfff)+PAGEMNGR_PAGE_SIZE)
#define PAGEMNGR_PAGE_ALIGN_DOWN(x)	(x &~ 0xfff)

int  pagemngr_init();
int  pagemngr_map_block(void *phys, void *virt);
void *pagemngr_unmap_block(void *virt);

#endif // _KERNEL_ARCH_I386_PAGEMNGR_H