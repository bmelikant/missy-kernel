#include <init/memory/kmemlow.h>
#include <init/multiboot/multiboot.h>
#include <init/kterm.h>
#include <init/kerrno.h>

#ifndef _cplusplus
#include <stdbool.h>
#endif

#define PAGE_SIZE 4096
#define KILOBYTES 1024
#define BLOCKS_PER_UNIT 32
#define FULL_BITMAP_UNIT 0xffffffff

extern unsigned int _mbitmap;
unsigned int *_mbitmap_physical;

static unsigned int total_blocks = 0;
static unsigned int used_blocks = 0;
static uint32_t bitmap_size = 0;
static void *_kernel_end_physical = NULL;

/** forward declarations of internal functions */
static void reserve_region(unsigned int start, unsigned int length);
static void free_region(unsigned int start, unsigned int length);
static unsigned int first_free();


static inline void allocate(unsigned int block) {
	unsigned int i = block / BLOCKS_PER_UNIT;
	unsigned int j = block % BLOCKS_PER_UNIT;
	_mbitmap_physical[i] |= (1<<j);

	#ifdef DEBUG_KMEMLOW
	unsigned int location = (unsigned int)(_mbitmap_physical+i);
	ki_printf("allocated in map at 0x%x\n", location);
	#endif
}

static inline void deallocate(unsigned int block) {
	unsigned int i = block / BLOCKS_PER_UNIT;
	unsigned int j = block % BLOCKS_PER_UNIT;
	_mbitmap_physical[i] &= ~(1<<j);

	#ifdef DEBUG_KMEMLOW
	//unsigned int location = (unsigned int)(_mbitmap_physical+i);
	//ki_printf("deallcoated in map at 0x%x\n", location);
	#endif
}

int kmemlow_init_allocator() {
    // locate the bitmap in physical memory
    unsigned int start_address = (unsigned int) &_mbitmap - 0xc0000000;
    _mbitmap_physical = (unsigned int *) start_address;
	#ifdef DEBUG_KMEMLOW
    ki_printf("The memory bitmap is located at physical address 0x%x\n", start_address);
	#endif

    // compute the size of the memory map; initialize everything to zero
    unsigned int memsz = multiboot_get_memsz();
    ki_printf("Total system memory: %d kilobytes\n", memsz);

    total_blocks = used_blocks = (memsz*KILOBYTES) / PAGE_SIZE;
	bitmap_size = total_blocks / BLOCKS_PER_UNIT;
	
	// round up in case the total blocks are not divisible by 32
	if (total_blocks%BLOCKS_PER_UNIT > 0) bitmap_size++;

	#ifdef DEBUG_KMEMLOW
	ki_printf("the bitmap contains %d unsigned int values\n", bitmap_size);
    ki_printf("block allocator has %d total blocks\n", total_blocks);
	ki_printf("all blocks will be marked as used until allocator is fully initialized\n");
	#endif

	// find a valid address at the end of the block allocator reserved region
	unsigned int _kend_phys = start_address + (bitmap_size*sizeof(unsigned int));
	_kend_phys &= ~0x00000fff;
	_kend_phys += PAGE_SIZE;
	_kernel_end_physical = (void *)(_kend_phys);
	
	#ifdef DEBUG_KMEMLOW
	ki_printf("_kend_phys: 0x%x\n", _kend_phys);
	ki_printf("the kernel's physical end address is 0x%x%x\n", (unsigned int)(_kernel_end_physical) >> 16, (unsigned int)(_kernel_end_physical) &~ 0xffff0000);
	#endif

	// grub2 multiboot data needs to be relocated beyond the end of the memory allocator >:(
	// for other multiboot-compliant loaders this probably won't be a problem so this calls a dummy method in those cases
	multiboot_relocate(_kernel_end_physical);
	for (unsigned int i = 0; i < bitmap_size; i++) {
		_mbitmap_physical[i] = 0xffffffff;
	}

	// loop over the memory map and free any blocks that are available
	mmap_data memory_map_block;
	while (multiboot_get_mmap_next(&memory_map_block) != -1) {
		if (memory_map_block.type == 1) {
			#ifdef DEBUG_KMEMLOW
			ki_printf("memory region - start: 0x%x, length: 0x%x\n", (uint32_t) memory_map_block.base, (uint32_t) memory_map_block.length);
			#endif
			free_region((uint32_t) memory_map_block.base,(uint32_t) memory_map_block.length);
		}
	}

	// we want to reserve the first 1MB and the kernel image memory
	// it'd be BAD to hand out our kernel as an allocated block!!
	unsigned int kernel_length = _kend_phys - 0x00100000;
	reserve_region(0x0, 0x100000);
	reserve_region(0x100000,kernel_length);

	#ifdef DEBUG_KMEMLOW
	ki_printf("total blocks: %d\n", total_blocks);
	ki_printf("used blocks: %d\n", used_blocks);
	ki_printf("free blocks: %d\n", total_blocks - used_blocks);
	#endif

	// that's it for the block allocator. we can just return success now
    return 0;
}

void *kmemlow_get_kernel_endptr() {
	return _kernel_end_physical;
}

void *kmemlow_get_bitmap_ptr() {
	return _mbitmap_physical;
}

uint32_t kmemlow_get_bitmap_size() {
	return bitmap_size;
}

uint32_t kmemlow_get_total_blocks() {
	return total_blocks;
}

uint32_t kmemlow_get_used_blocks() {
	return used_blocks;
}

void *kmemlow_alloc() {
	uint32_t free_block = first_free();
	#ifdef DEBUG_KMEMLOW
	ki_printf("found free block: 0x%x\n", free_block);
	#endif

	if (free_block == 0 && kinit_errno == KMEMLOW_ERROR_ENOMEM) {
		return NULL;
	}

	allocate(free_block);
	void *allocated_ptr = (void *)(free_block*PAGE_SIZE);
	return allocated_ptr;
}


void kmemlow_free(void *block) {
	uint32_t allocated_block = (uint32_t)(block) / PAGE_SIZE;
	#ifdef DEBUG_KMEMLOW
	ki_printf("allocated block is: 0x%x\n", allocated_block);
	#endif

	// make sure the block isn't outside the block map
	if (allocated_block < total_blocks) {
		deallocate(allocated_block);
		#ifdef DEBUG_KMEMLOW
		ki_printf("deallocated block at address 0x%x\n", allocated_block);
		#endif
	} else {
		#ifdef DEBUG_KMEMLOW
		kernel_early_puts("block is outside the range of available blocks:");
		ki_printf("block: %d, total blocks: %d\n", allocated_block, total_blocks);
		#endif
	}
}

/** internal functions */

void reserve_region(unsigned int base, unsigned int length) {
	// convert the start address to a page-aligned value
	unsigned int align = base / PAGE_SIZE;
	unsigned int blocks = length / PAGE_SIZE;
	
	#ifdef DEBUG_KMEMLOW
	ki_printf("reserve - align: 0x%x, blocks: 0x%x\n", align, blocks);
	#endif
	while (blocks > 0) {	
		allocate(align++);
		blocks--;
		used_blocks++;
	}
}

void free_region(unsigned int base, unsigned int length) {
	unsigned int align = base / PAGE_SIZE;
	unsigned int blocks = length / PAGE_SIZE;
	#ifdef DEBUG_KMEMLOW
	ki_printf("free - align: 0x%x, blocks: 0x%x\n", align, blocks);
	#endif
	while (blocks > 0) {
		deallocate(align++);
		blocks--;
		used_blocks--;
	}
}

unsigned int first_free() {
	for (size_t i = 0; i < total_blocks / BLOCKS_PER_UNIT; i++) {
		if (_mbitmap_physical[i] != FULL_BITMAP_UNIT) {
			for (size_t j = 0; j < BLOCKS_PER_UNIT; j++) {
				if (((_mbitmap_physical[i] >> j) & 1) == 0) {
					return (i*BLOCKS_PER_UNIT)+j;
				}
			}
		}
	}
	kinit_errno = KMEMLOW_ERROR_ENOMEM;
	return 0;
}