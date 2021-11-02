#include <init/memory/kmemlow.h>
#include <init/memory/paging.h>
#include <init/kutils.h>
#include <init/kerrno.h>
#include <init/kterm.h>

#include "../../kernel/include/kernel/bochsdbg.h"

#define PTABLE_FLAG_PRESENT 		0x0001
#define PTABLE_FLAG_RW				0x0002
#define PTABLE_FLAG_USER			0x0004
#define PTABLE_FLAG_WRITETHROUGH	0x0008
#define PTABLE_FLAG_NOCACHE			0x0010
#define PTABLE_FLAG_ACCESSED		0x0020
#define PTABLE_FLAG_DIRTY			0x0040
#define PDIR_FLAG_PAGE_SIZE			0x0080
#define PTABLE_FLAG_GLOBAL			0x0100

#define pdir_entry(x) (x >> 22)
#define ptable_entry(x) ((x & ~0xffc00000) >> 12)

#define IDENTITY_MAP_ENTRY 0
#define KERNEL_MAP_ENTRY
#define PAGE_SIZE 4096

#define PAGE_DIRECTORY_MAPPING 	0xffc00000
#define KERNEL_STACK_MAPPING	(PAGE_DIRECTORY_MAPPING-PAGE_SIZE)

uint32_t *_page_directory = NULL;

static uint32_t *_first_pagetable = NULL;
static uint32_t *_kernel_pagetable = NULL;
static uint32_t *_kernel_stack_map = NULL;

static void *_kernel_end_virtual = NULL;

/** internal functions */
static int create_page_directory();
static int map_first_megabyte();
static int map_kernel();
static int create_new_kernel_stack();

int ki_setup_paging() {
	int result = create_page_directory();
	if (result != 0) {
		return result;
	}

	result = map_first_megabyte();
	if (result != 0) {
		return result;
	}

	result = map_kernel();
	if (result != 0) {
		return result;
	}

	result = create_new_kernel_stack();
	if (result != 0) {
		return result;
	}

	return 0;
}

void *ki_get_kernel_end_virtual() {
	return _kernel_end_virtual;
}

void *ki_get_kernel_stack_top() {
	return (void*)((KERNEL_STACK_MAPPING+PAGE_SIZE)-4);
}

int create_page_directory() {
	#ifdef DEBUG_BUILD
	ki_printf("creating page directory...\n");
	#endif

	// allocate the first page directory and map it to 0xffc00000
	_page_directory = (uint32_t *) kmemlow_alloc();
	if (!_page_directory) {
		kinit_errno = ERROR_PAGING_ENOMEM;
		return -1;
	}

	#ifdef DEBUG_BUILD
	ki_printf("page directory address: 0x%x\n", (uint32_t) _page_directory);
	#endif

	ki_memset(_page_directory,0,PAGE_SIZE);
	_page_directory[pdir_entry(0xffc00000)] = (uint32_t)(_page_directory) | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;
	//BOCHS_MAGIC_BREAKPOINT();
	return 0;
}

int map_first_megabyte() {

	#ifdef DEBUG_BUILD
	ki_printf("mapping first megabyte...\n");
	#endif

	_first_pagetable = (uint32_t *) kmemlow_alloc();
	if (!_first_pagetable) {
		kinit_errno = ERROR_PAGING_ENOMEM;
		return -1;
	}

	#ifdef DEBUG_BUILD
	ki_printf("page table address: 0x%x\n", (uint32_t) _first_pagetable);
	#endif

	ki_memset(_first_pagetable,0,PAGE_SIZE);
	_page_directory[pdir_entry(0)] = (uint32_t)(_first_pagetable) | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;

	for (uint32_t i = 0; i < 0x100000; i += PAGE_SIZE) {
		_first_pagetable[ptable_entry(i)] = i | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;
	}
	//BOCHS_MAGIC_BREAKPOINT();
	return 0;
}

int map_kernel() {

	#ifdef DEBUG_BUILD
	ki_printf("mapping kernel...");
	#endif

	_kernel_pagetable = (uint32_t *) kmemlow_alloc();
	if (!_kernel_pagetable) {
		kinit_errno = ERROR_PAGING_ENOMEM;
		return -1;
	}

	#ifdef DEBUG_BUILD
	ki_printf("kernel page table address: 0x%x\n", (uint32_t) _kernel_pagetable);
	#endif

	ki_memset(_kernel_pagetable,0,PAGE_SIZE);
	_page_directory[pdir_entry(0xc0100000)] = (uint32_t)(_kernel_pagetable) | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;
	_kernel_end_virtual = kmemlow_get_kernel_endptr() + 0xc0000000;

	ki_printf("kernel end virtual = 0x%x\n", (uint32_t) _kernel_end_virtual);
	for (uint32_t i = 0xc0100000; i < (uint32_t)(_kernel_end_virtual); i += PAGE_SIZE) {
		_kernel_pagetable[ptable_entry(i)] = (i - 0xc0000000) | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;
	}
	//BOCHS_MAGIC_BREAKPOINT();
	return 0;
}

int create_new_kernel_stack() {
	// kernel stack should start just below the page directory mapping
	_kernel_stack_map = (uint32_t *) kmemlow_alloc();
	if (!_kernel_stack_map) {
		kinit_errno = ERROR_PAGING_ENOMEM;
		return -1;
	}

	void *_kernel_stack_first_block = kmemlow_alloc();
	if (!_kernel_stack_first_block) {
		kinit_errno = ERROR_PAGING_ENOMEM;
		return -1;
	}

	ki_memset(_kernel_stack_map,0,PAGE_SIZE);
	ki_memset(_kernel_stack_first_block,0,PAGE_SIZE);
	_page_directory[pdir_entry(KERNEL_STACK_MAPPING)] = (uint32_t)(_kernel_stack_map) | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;
	_kernel_stack_map[ptable_entry(KERNEL_STACK_MAPPING)] = (uint32_t)(_kernel_stack_first_block) | PTABLE_FLAG_PRESENT | PTABLE_FLAG_USER;
	return 0;
}