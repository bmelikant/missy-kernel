#ifndef _KERNEL_ARCH_I386_PAGEMNGR_H
#define _KERNEL_ARCH_I386_PAGEMNGR_H 1

#define PAGE_SIZE			4096
#define ALIGN_BLOCK_UP(x)	((x &~ 0xfff)+PAGE_SIZE)
#define ALIGN_BLOCK_DOWN(x)	(x &~ 0xfff)

int  pagemngr_init();
int  pagemngr_map_block(void *phys, void *virt);
void *pagemngr_unmap_block(void *virt);

#endif // _KERNEL_ARCH_I386_PAGEMNGR_H