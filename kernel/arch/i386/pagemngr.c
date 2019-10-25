/**
 * Manage the mapping of physical pages to virtual (flat) addresses
 * in the kernel address space
 */

#include <stdint.h>
#include <stddef.h>

#include "pagemngr.h"
#include "balloc.h"

#define PAGE_DIRECTORY_VIRTUAL_BASE	0xfffff000
#define PAGE_TABLE_VIRTUAL_BASE		0xffc00000

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

uint32_t *_page_directory_physical = NULL;
uint32_t *_page_directory_virtual  = (uint32_t *) PAGE_DIRECTORY_VIRTUAL_BASE;

static inline void set_pdir_base_register(void *pdir_physical) { 
	asm volatile("mov %%cr3;%0\n" :: "b"(pdir_physical)); 
}

static inline void *get_pdir_base_register(void) { 
	void *pdir_base = NULL;
	asm volatile("mov %0,%%cr3\n" : "=r"(pdir_base));
	return pdir_base;
}

static inline void invalidate_page(void *page_entry) {
	asm("invlpg %0" : : "m"((char*)page_entry) : "memory");

}

int pagemngr_init() {
	// paging is already enabled, so get the kernel page directory from CR3
	_page_directory_physical = (uint32_t *) get_pdir_base_register();
	return 0;
}

int pagemngr_map_block(void *physical, void *virtual) {
	uint32_t page_table_entry = (uint32_t)(physical) | PTABLE_FLAG_PRESENT;
	uint32_t page_directory_entry = _page_directory_virtual[pdir_entry((uint32_t)virtual)];

	// this page table is unmapped; map it
	if (page_directory_entry == 0) {
		void *new_page_table = balloc_allocate_block();
		if (!new_page_table) return -1;

		_page_directory_virtual[pdir_entry((uint32_t) virtual)] = (uint32_t)(new_page_table) | PTABLE_FLAG_PRESENT;
	}

	// compute the virtual address of the destination page
	uint32_t *page_table_virtual = (uint32_t *)(PAGE_TABLE_VIRTUAL_BASE + (pdir_entry((uint32_t) virtual)*PAGE_SIZE));
	if (page_table_virtual[ptable_entry((uint32_t) virtual)] != 0) {
		// something is already mapped here...
		return -1;
	}

	page_table_virtual[ptable_entry((uint32_t) virtual)] = page_table_entry;
	invalidate_page(virtual);
	return 0;
}

void *pagemngr_unmap_block(void *virtual) {
	void *physical_address = NULL;

	if (_page_directory_virtual[pdir_entry((uint32_t) virtual)] & PTABLE_FLAG_PRESENT) {
		// compute the entry in the page table
		uint32_t *page_table_virtual = (uint32_t *)(PAGE_TABLE_VIRTUAL_BASE + (pdir_entry((uint32_t) virtual)*PAGE_SIZE));
		if (page_table_virtual[ptable_entry((uint32_t) virtual)] & PTABLE_FLAG_PRESENT) {
			physical_address = (void *)((page_table_virtual[ptable_entry((uint32_t) virtual)])&0xfff); 
			page_table_virtual[ptable_entry((uint32_t) virtual)] = 0;
			
			// if this page directory entry is completely empty, unmap it
			int i = 0;
			for (; i < (int)(PAGE_SIZE / sizeof(uint32_t)); i++) {
				if (page_table_virtual[i] > 0) {
					break;
				}
			}

			if (i >= (int)(PAGE_SIZE / sizeof(uint32_t))) {
				balloc_deallocate_block((void *)(_page_directory_virtual[pdir_entry((uint32_t) virtual)]));
				_page_directory_virtual[pdir_entry((uint32_t) virtual)] = 0;
			}
			invalidate_page(virtual);
		}
	}
	return physical_address;
}