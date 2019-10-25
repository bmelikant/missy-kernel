#include <init/multiboot/multiboot.h>
#include <init/kterm.h>
#include <init/kutils.h>
#include <init/kerrno.h>

#include <limits.h>

#define MBOOT2_TYPETAG_END		0x00
#define MBOOT2_TYPETAG_MEMSZ	0x04
#define MBOOT2_TYPETAG_MMAP		0x06
#define MBOOT2_TYPETAG_BLNAME	0x02
#define MBOOT2_TYPETAG_CMDLINE	0x01
#define MBOOT2_TYPETAG_MODULES	0x03
#define MBOOT2_TYPETAG_ELFSYM	0x09
#define MBOOT2_TYPETAG_ACPI_1	0x0E
#define MBOOT2_TYPETAG_ACPI_2	0x0F

typedef struct MULTIBOOT2_TAG 			multiboot2_tag;
typedef struct MULTIBOOT2_TAG_MEMSZ 	multiboot2_tag_memsz;
typedef struct MULTIBOOT2_ENTRY_MMAP 	multiboot2_entry_mmap;
typedef struct MULTIBOOT2_TAG_MMAP 		multiboot2_tag_mmap;

/* base multiboot tag. every tag starts with this entry */
struct MULTIBOOT2_TAG {

	uint32_t type;
	uint32_t size;

}__attribute__((packed));

/* memory size tag. contains info about how much memory is available */
struct MULTIBOOT2_TAG_MEMSZ {

	struct MULTIBOOT2_TAG tag;
	uint32_t mem_lower;
	uint32_t mem_upper;

}__attribute__((packed));

/* memory map entry structure for multiboot2 */
struct MULTIBOOT2_ENTRY_MMAP {

	uint64_t base_addr;
	uint64_t length;
	uint32_t type;
	uint32_t reserved;

}__attribute__((packed));

/* memory map tag. contains info on memory layout for the system. */
struct MULTIBOOT2_TAG_MMAP {

	struct MULTIBOOT2_TAG tag;
	uint32_t entry_sz;
	uint32_t entry_ver;
	struct MULTIBOOT2_ENTRY_MMAP entries[0];

}__attribute__((packed));

typedef struct MULTIBOOT2_TAG_CHECK {
	unsigned int tag;
	unsigned int errno;
} multiboot2_tagcheck_t;

typedef struct MULTIBOOT2_TAG_ACPI1 {
	struct MULTIBOOT2_TAG tag;
	_acpi_rsdp_t acpi_tag;
}__attribute__((packed)) multiboot2_tag_acpi;

typedef struct MULTIBOOT2_TAG_ACPI2 {
	struct MULTIBOOT2_TAG tag;
	_acpi2_rsdp_t acpi_tag;
}__attribute__((packed)) multiboot2_tag_acpi2;

static multiboot2_tagcheck_t multiboot2_required[] = {
	{ MBOOT2_TYPETAG_MEMSZ, MBOOT_ERROR_INVALID_MEMSIZE },
	{ MBOOT2_TYPETAG_MMAP, MBOOT_ERROR_INVALID_MEMORY_MAP }
};

/** implemented in api.c */
extern unsigned int boot_magic;
extern unsigned int mb_memory_sz;
extern void *mboot_ptr;
extern void *mmap_ptr;

static unsigned int current_mmap_entry = 0;

/** internal function declarations */
static void multiboot2_install_api_fns(multiboot_api_t *api_struct);
static int check_multiboot2_tags();
static void *mboot2_find_tag(unsigned int tag_type);
static multiboot2_entry_mmap *mboot2_next_mmap();
size_t mboot2_get_structure_length();


/** Multiboot2 functions */
int init_multiboot2(multiboot_api_t *api) {
	multiboot2_install_api_fns(api);
	return check_multiboot2_tags();
}

static int mboot2_get_memory_size(void *buffer) {
	unsigned int *memsz_buffer = (unsigned int *) buffer;
	multiboot2_tag_memsz *memsz_tag = (multiboot2_tag_memsz *)mboot2_find_tag(MBOOT2_TYPETAG_MEMSZ);
	if (!memsz_tag) {
		kinit_errno = MBOOT_ERROR_ENTRY_NOT_FOUND;
		return -1;
	}

	*memsz_buffer = (unsigned int)(memsz_tag->mem_lower+memsz_tag->mem_upper);
	return 0;
}

static int mboot2_get_next_mmap(void *buffer) {
	#ifdef DEBUG_MULTIBOOT
	ki_printf("getting memory map entry %d\n", current_mmap_entry);
	#endif
	mmap_data *mmap_buffer = (mmap_data *) buffer;
	multiboot2_entry_mmap *next = mboot2_next_mmap();
	if (!next) {
		#ifdef DEBUG_MULTIBOOT
		if (kinit_errno == MBOOT_ERROR_ENTRY_NOT_FOUND) kernel_early_puts("The memory map could not be located\n");
		else ki_printf("Reached end of memory map\n");
		#endif
		return -1;
	}

	mmap_buffer->base = next->base_addr;
	mmap_buffer->length = next->length;
	mmap_buffer->type = next->type;
	#ifdef DEBUG_MULTIBOOT2
	ki_printf("type: 0x%x\n", mmap_buffer->type);
	#endif

	return 0;
}

static int mboot2_relocate_tags(void *kernel_end) {
	#ifdef DEBUG_MULTIBOOT2
	unsigned int oldloc = (unsigned int) mboot_ptr;
	unsigned int newloc = (unsigned int) kernel_end;
	ki_printf("relocating multiboot from 0x%x to 0x%x\n", oldloc, newloc);
	#endif
	// copy the multiboot struct above the end of the kernel. we want to make sure the multiboot data is dword-aligned
	// to be safe here, we are going to add the length of the boot struct to the new kernel end pointer
	// just in case the initial memory overlaps. Verified as an issue on systems with 128mb or less
	size_t mboot_struct_size = mboot2_get_structure_length();
	kernel_end = (void *)(kernel_end + mboot_struct_size);

	#ifdef DEBUG_MULTIBOOT2
	ki_printf("multiboot structure is %d bytes in length\n", mboot_struct_size);
	#endif

	ki_memcpy(kernel_end,mboot_ptr,mboot_struct_size);
	mboot_ptr = kernel_end;

	return 0;
}

/** internal functions */

int check_multiboot2_tags() {
	#ifdef DEBUG_MULTIBOOT
	ki_printf("checking multiboot tags...\n");
	#endif
	size_t required_tag_count = sizeof(multiboot2_required) / sizeof(multiboot2_tagcheck_t);
	for (size_t i = 0; i < required_tag_count; i++) {
		#ifdef DEBUG_MULTIBOOT
		ki_printf("Checking for multiboot tag: %d\n", multiboot2_required[i].tag);
		#endif
		void *current_tag = mboot2_find_tag(multiboot2_required[i].tag);
		if (!current_tag) {
			#ifdef DEBUG_MULTIBOOT
			ki_printf("Tag %d was not found\n", multiboot2_required[i].tag);
			#endif
			// tag wasn't found; was it due to an invalid multiboot header?
			if (kinit_errno != MBOOT_ERROR_INVALID_BOOTSTRUCT) {
				#ifdef DEBUG_MULTIBOOT
				ki_printf("Cause: %d\n", multiboot2_required[i].errno);
				#endif
				kinit_errno = multiboot2_required[i].errno;
			}
			return -1;
		} else {
			#ifdef DEBUG_MULTIBOOT
			ki_printf("Tag %d was found\n", multiboot2_required[i].tag);
			#endif
		}
	}
	return 0;
}

// void *mboot2_find_tag (): Locate the given tag within the multiboot2 tag list
// inputs: tag_type - tag type to find
// returns: a pointer to the tag in memory, NULL if tag isn't found
void *mboot2_find_tag(unsigned int tag_type) {
	// make sure the multiboot2 data exists!!
	multiboot2_tag *multiboot_start = (multiboot2_tag*) mboot_ptr;
	if (!multiboot_start) {
		kinit_errno = MBOOT_ERROR_INVALID_BOOTSTRUCT;
		return NULL;
	}

	// multiboot is weird; need to make this an unsigned log for this to work right!
	unsigned long addr = (unsigned long)(multiboot_start);
	#ifdef DEBUG_MULTIBOOT2
	ki_printf("addr: 0x%x\n", (unsigned int) addr);
	#endif
	// okay, loop thru the multiboot2 tags until we find the right one
	for (multiboot2_tag *tag = (multiboot2_tag*) (addr+8);
			tag->type != MBOOT2_TYPETAG_END;
			tag = (multiboot2_tag *) ((uint8_t *) tag + ((tag->size + 7) & ~7))) {
		// if the tag type matches what we are looking for, return a pointer to the tag
		if (tag->type == tag_type) {
			#ifdef DEBUG_MULTIBOOT2
			ki_printf("found tag type 0x%x\n", tag->type);
			#endif
			return (void *) tag;
		}
	}

	kinit_errno = MBOOT_ERROR_ENTRY_NOT_FOUND;
	return NULL;
}

// get the next multiboot2 memory map entry
multiboot2_entry_mmap *mboot2_next_mmap() {
	
	// get the memory map entry count
	multiboot2_tag_mmap *memory_map_ptr = (multiboot2_tag_mmap *) mboot2_find_tag(MBOOT2_TYPETAG_MMAP);
	if (!memory_map_ptr) {
		kinit_errno = MBOOT_ERROR_ENTRY_NOT_FOUND;
		return NULL;
	}

	unsigned int mmap_entries = ((memory_map_ptr->tag.size-sizeof(multiboot2_tag_mmap)) / memory_map_ptr->entry_sz);
	#ifdef DEBUG_MULTIBOOT2
	unsigned int mmap_ptr_value = (unsigned int) memory_map_ptr;
	ki_printf("first mmap_entry: 0x%x\n", mmap_ptr_value);
	ki_printf("mmap_entries: 0x%x\n", mmap_entries);
	#endif
	// locate the memory map entry
	if (current_mmap_entry < mmap_entries) {
		multiboot2_entry_mmap *entry = (multiboot2_entry_mmap *)((unsigned int) memory_map_ptr+sizeof(multiboot2_tag_mmap)+
				(sizeof(multiboot2_entry_mmap)*current_mmap_entry));

		#ifdef DEBUG_MULTIBOOT2
		unsigned int entryaddr = (unsigned int) entry;
		ki_printf("current mmap entry is at 0x%x\n", entryaddr);
		#endif

		current_mmap_entry++;
		return entry;
	}

	return NULL;
}

 size_t mboot2_get_structure_length() {
	multiboot2_tag *multiboot2_start = (multiboot2_tag *) mboot_ptr;
	if (!multiboot2_start) {
		kinit_errno = MBOOT_ERROR_INVALID_BOOTSTRUCT;
		return 0;
	}

	unsigned long addr = (unsigned long) multiboot2_start;
	uint32_t total_size = *(unsigned *) addr;
	return (size_t) total_size;
}

int mboot2_get_acpi_rsdp(void *rsdp2ptr) {
	_acpi2_rsdp_t *destination = (_acpi2_rsdp_t *) rsdp2ptr;
	multiboot2_tag *tag = mboot2_find_tag(MBOOT2_TYPETAG_ACPI_1);
	ki_memset(destination,0,sizeof(_acpi2_rsdp_t));

	if (!tag) {
		tag = mboot2_find_tag(MBOOT2_TYPETAG_ACPI_2);
		if (!tag) {
			#ifdef DEBUG_MULTIBOOT
			kernel_early_puts("Could not find ACPI tag");
			#endif
			return -1;
		}
		multiboot2_tag_acpi2 *acpi2_tag = (multiboot2_tag_acpi2 *)(tag);
		ki_memcpy(destination,&acpi2_tag->acpi_tag,sizeof(_acpi2_rsdp_t));
	} else {
		multiboot2_tag_acpi *acpi_tag = (multiboot2_tag_acpi *)(tag);
		ki_memcpy(destination,&acpi_tag->acpi_tag,sizeof(_acpi_rsdp_t));
	}

	return 0;
}

void multiboot2_install_api_fns(multiboot_api_t *api_struct) {
	#ifdef DEBUG_MULTIBOOT
	ki_printf("installing API functions...\n");
	#endif
	api_struct->get_memory_size = mboot2_get_memory_size;
	api_struct->get_next_mmap_entry = mboot2_get_next_mmap;
	api_struct->relocate_multiboot = mboot2_relocate_tags;
	api_struct->get_rsdp = mboot2_get_acpi_rsdp;
}