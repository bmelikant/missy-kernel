/*
 * multiboot.c: Functions for parsing information in the
 * multiboot and multiboot2 standards
 *
 *  Created on: Aug 21, 2017
 *      Author: bmelikant
 */

#include <multiboot.h>
#include <early_errno.h>
#include <early_terminal.h>

// internal data
static unsigned int boot_magic = 0;
static unsigned int mb_memory_sz = 0;

// multiboot internal data
static multiboot_info *mb_inf = NULL;
static memory_map_inf *mb_memory_map = NULL;
static size_t multiboot_cur_mmap = 0;

// multiboot2 internal data
static multiboot2_tag *mb2_start = NULL;
static multiboot2_tag_mmap *mb2_memory_map = NULL;

/** multiboot internal function declarations */
static memory_map_inf *multiboot_next_mmap ();

/** multiboot2 internal function declarations */
static void 					*mboot2_find_tag (unsigned int tag_type);
static multiboot2_entry_mmap 	*mboot2_next_mmap ();

/** 
 * @fn int multiboot_data_init (void *, unsigned int): Initialize the multiboot data reader
 * @param mb_ptr - pointer to the start of the multiboot struct
 * @param magic - defines multiboot type
 * @return 0 on success, -1 on error. This function sets early_errno
 */
int multiboot_data_init (void *mb_ptr, unsigned int magic) {
	// check to make sure the multiboot ptr is word-aligned
	if ((unsigned long) mb_ptr & 7) {
		early_errno = MBOOT_ERROR_MISALIGNED_HEADER;
		return -1;
	}

	// save the magic field
	boot_magic = magic;

	if (boot_magic == MULTIBOOT_MAGIC) {
		// store the multiboot info structure
		mb_inf = (multiboot_info *) mb_ptr;
		// at minimum, we must have the memory map and memory size flags present
		if (!bootflag_check (mb_inf->flags, MEMSZ_PRESENT)) {
			early_errno = MBOOT_ERROR_INVALID_MEMSIZE;
			return -1;
		}

		mb_memory_sz = (unsigned int)(mb_inf->memorySzLo+mb_inf->memorySzHi);
		if (!bootflag_check (mb_inf->flags, MMAP_PRESENT)) {
			early_errno = MBOOT_ERROR_INVALID_MEMORY_MAP;
			return -1;
		}

		mb_memory_map = (memory_map_inf *) mb_inf->mmap_addr;
		return 0;
	
	} else if (boot_magic == MULTIBOOT2_MAGIC) {
		// display the multiboot2 header size
		unsigned int size = *(unsigned *) mb_ptr;
		// store the first multiboot2 tag
		mb2_start = (multiboot2_tag *) mb_ptr;
		// find the memory size tag
		multiboot2_tag_memsz *msize = (multiboot2_tag_memsz *) mboot2_find_tag(MBOOT2_TYPETAG_MEMSZ);
		
		// the tag doesn't exist... eew
		if (!msize) {
			early_errno = MBOOT_ERROR_INVALID_MEMSIZE;
			return -1;
		}

		// compute the size from the tag
		mb_memory_sz = (unsigned int)(msize->mem_lower+msize->mem_upper);
		// look for the memory map tag
		mb2_memory_map = (multiboot2_tag_mmap *) mboot2_find_tag(MBOOT2_TYPETAG_MMAP);

		if (!mb2_memory_map) {
			early_errno = MBOOT_ERROR_INVALID_MEMORY_MAP;
			return -1;
		}

		return 0;
	}

	// unrecognized, non-compliant loader
	return -1;
}

/**
 * unsigned int multiboot_get_memsz (): Get the size of physical memory from multiboot
 * inputs: none
 * returns: size of memory in KB
 */
unsigned int multiboot_get_memsz () {
	return mb_memory_sz;
}

/**
 * mmap_data *multiboot_get_mmap_next (): Get the next entry from the memory map
 * inputs: buf - the buffer for the entry data
 * returns: * to buf (NULL if at end of list)
 */
mmap_data *multiboot_get_mmap_next(mmap_data *buf) {
	// grab the next item
	if (boot_magic == MULTIBOOT_MAGIC) {

		memory_map_inf *entry = multiboot_next_mmap();
		if (!entry) {
			return NULL;
		}

		// copy the data over to the buffer
		buf->base = entry->baseAddr;
		buf->length = entry->length;
		buf->type = entry->type;
		return buf;
	
	} else if (boot_magic == MULTIBOOT2_MAGIC) {

		multiboot2_entry_mmap *entry = mboot2_next_mmap();
		if (!entry) {
			return NULL;
		}
		// copy the data over to the buffer
		buf->base = entry->base_addr;
		buf->length = entry->length;
		buf->type = entry->type;
		return buf;
	}

	// somehow the magic value was corrupted; flag this to the OS
	early_errno = MBOOT_ERROR_NONCOMPLIANT_LOADER;
	return NULL;
}

// for multiboot2 loaders: find the pointer to ACPI RDSP
void *multiboot_find_rdsp() {
	// ACPI data only exists for multiboot2
	if (boot_magic == MULTIBOOT2_MAGIC) {
		early_errno = MBOOT_NOT_IMPLEMENTED;
		return NULL;
	}

	early_errno = MBOOT_ERROR_UNSUPPORTED;
	return NULL;
}

/* multiboot internal functions */

memory_map_inf *multiboot_next_mmap() {

	// grab the next memory map entry
	unsigned int mmap_entries = (unsigned int)(mb_inf->mmap_length / sizeof (memory_map_inf));

	// make sure we are within the range of entries
	if (multiboot_cur_mmap < mmap_entries) {

		memory_map_inf *entry = (memory_map_inf *)((unsigned int) mb_memory_map+mb_memory_map->sz+sizeof(mb_memory_map->sz)+
				(multiboot_cur_mmap*sizeof(memory_map_inf)));

		multiboot_cur_mmap++;
		return entry;
	}

	return NULL;
}

/* multiboot2 internal functions */

// void *mboot2_find_tag (): Locate the given tag within the multiboot2 tag list
// inputs: tag_type - tag type to find
// returns: a pointer to the tag in memory, NULL if tag isn't found
void *mboot2_find_tag(unsigned int tag_type) {
	// make sure the multiboot2 data exists!!
	if (!mb2_start) {
		early_errno = MBOOT_ERROR_INVALID_BOOTSTRUCT;
		return NULL;
	}

	// multiboot is weird; need to make this an unsigned log for this to work right!
	unsigned long addr = (unsigned long) mb2_start;
	// okay, loop thru the multiboot2 tags until we find the right one
	for (multiboot2_tag *tag = (multiboot2_tag*) (addr+8);
			tag->type != MBOOT2_TYPETAG_END;
			tag = (multiboot2_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7))) {

		// if the tag type matches what we are looking for, return a pointer to the tag
		if (tag->type == tag_type) {
			return (void *) tag;
		}
	}

	early_errno = MBOOT_ERROR_ENTRY_NOT_FOUND;
	return NULL;
}

// get the next multiboot2 memory map entry
multiboot2_entry_mmap *mboot2_next_mmap () {

	// get the memory map entry count
	unsigned int mmap_entries = ((mb2_memory_map->tag.size-sizeof(multiboot2_tag_mmap)) / mb2_memory_map->entry_sz);

	// locate the memory map entry
	if (multiboot_cur_mmap < mmap_entries) {
		multiboot2_entry_mmap *entry = (multiboot2_entry_mmap *)((unsigned int) mb2_memory_map+sizeof(multiboot2_tag_mmap)+
				(sizeof(multiboot2_entry_mmap)*multiboot_cur_mmap));

		multiboot_cur_mmap++;
		return entry;
	}

	return NULL;
}
