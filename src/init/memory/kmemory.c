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

    total_blocks = (memsz*KILOBYTES) / PAGE_SIZE;
#ifdef DEBUG_BUILD
    kernel_early_printf("Block allocator has %d total blocks", total_blocks);
#endif

    return 0;
}