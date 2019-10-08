#include <init/memory/kmemory.h>
#include <init/multiboot/multiboot.h>
#include <init/kterm.h>

#define PAGE_SIZE 4096
#define KILOBYTES 1024

extern unsigned int _mbitmap;
unsigned int *_mbitmap_physical;

static unsigned int total_blocks = 0;
static unsigned int used_blocks = 0;

int kmemory_init_allocator() {
    // locate the bitmap in physical memory
    unsigned int address = (unsigned int) &_mbitmap - 0xc0000000;
    _mbitmap_physical = (unsigned int *) address;
	#ifdef DEBUG_BUILD
    kernel_early_printf("The memory bitmap is located at physical address 0x%x\n", address);
	#endif

    // compute the size of the memory map; initialize everything to zero
    unsigned int memsz = multiboot_get_memsz();
    kernel_early_printf("Total system memory: %d kilobytes\n", memsz);

    total_blocks = used_blocks = (memsz*KILOBYTES) / PAGE_SIZE;
	unsigned int bitmap_sz = total_blocks / 32;
	#ifdef DEBUG_BUILD
	kernel_early_printf("the bitmap contains %d unsigned int values\n", bitmap_sz);
    kernel_early_printf("block allocator has %d total blocks\n", total_blocks);
	kernel_early_printf("all blocks will be marked as used until allocator is fully initialized\n");
	#endif

	// find a valid address at the end of the block allocator reserved region
	unsigned int _kend_phys = (unsigned int) &_mbitmap_physical + (total_blocks / sizeof(unsigned int));
	void *_kernel_end_physical = (void *)(_kend_phys);
	
	#ifdef DEBUG_BUILD
	kernel_early_printf("the kernel's physical end address is 0x%x%x\n", (unsigned int)(_kernel_end_physical) >> 16, (unsigned int)(_kernel_end_physical) &~ 0xffff0000);
	#endif

	// initialize all blocks to used
	//for (unsigned int i = 0; i < total_blocks; i++) {
		//_mbitmap_physical[i] = 0xffffffff;
	//}

	// loop over the memory map and free any blocks that are available
	/**
	mmap_data memory_map_block;
	while (multiboot_get_mmap_next(&memory_map_block) != -1) {
		kernel_early_printf("\tbase address: 0x%x%x\n", memory_map_block.base);
		kernel_early_printf("\tlength: 0x%x%x\n", memory_map_block.length);
		kernel_early_printf("\ttype: %d\n\n", memory_map_block.type);
	}
	*/

    return 0;
}