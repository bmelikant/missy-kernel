#include <init/multiboot/multiboot.h>
#include <init/kterm.h>
#include <init/kutils.h>
#include <init/kerrno.h>

// multiboot_info structure

struct MULTIBOOT_INFO {

	uint32_t flags;					// this field is required. Structure flags
	uint32_t memorySzLo;			// memory size (if flags[0] is set)
	uint32_t memorySzHi;
	uint32_t bootDevice;			// boot device (if flags[1] is set)
	uint32_t commandLine;			// kernel command line (if flags[2] is set)
	uint32_t mods_count;			// # of modules loaded (if flags[3] is set)
	uint32_t mods_addr;				// address of modules (if flags[3] is set)
	uint32_t elf_symbol_count;		// how many section symbols are present?
	uint32_t elf_entry_sz;			// how big is each symbol table entry?
	uint32_t elf_table_addr;		// where is the ELF symbol table located?
	uint32_t elf_string_table;		// strings representing ELF symbols
	uint32_t mmap_length;			// length of memory map (if flags[6] is set)
	uint32_t mmap_addr;				// address of memory map
	uint32_t drives_length;			// address & length of first drive structure
	uint32_t drives_addr;
	uint32_t config_table;			// ROM configuration table
	uint32_t bootldr_name;			// name of bootloader
	uint32_t apm_table;				// Advanced power management table
	uint32_t vbe_ctrl_info;			// information for video mode
	uint32_t vbe_mode_info;
	uint16_t vbe_mode;
	uint16_t vbe_interface_seg;
	uint16_t vbe_interface_off;
	uint16_t vbe_interface_len;

}__attribute__((packed));

// GRUB memory map structure info

struct MEMORY_MAP_INF {

        uint32_t sz;            // size of this structure
        uint64_t baseAddr;      // base address of the memory region
        uint64_t length;        // memory section length
        uint32_t type;          // type of the region
 
}__attribute__((packed));

// GRUB boot modules structure

struct MBOOT_MODULE_INF {

	unsigned int 	mod_addr_start;
	unsigned int 	mod_addr_end;
	const char 	   *mod_string;

}__attribute__((packed));

/* multiboot typedefs */
typedef struct MULTIBOOT_INFO multiboot_info;
typedef struct MEMORY_MAP_INF memory_map_inf;
typedef struct MBOOT_MODULE_INF mboot_module_inf;

typedef struct MULTIBOOT_FLAG_CHECK {
	uint32_t flag;
	unsigned int errno;
} multiboot_flagcheck_t;


static multiboot_flagcheck_t multiboot_required[] = {
	{ MEMSZ_PRESENT, MBOOT_ERROR_INVALID_MEMSIZE },
	{ MMAP_PRESENT, MBOOT_ERROR_INVALID_MEMORY_MAP }
};

extern unsigned int boot_magic;
extern unsigned int mb_memory_sz;
extern void *mboot_ptr;
extern void *mmap_ptr;

static unsigned int current_mmap_entry = 0;

/** internal functions */
static int check_multiboot_flags(uint32_t flags);
static void multiboot_install_api_fns(multiboot_api_t *api);
static memory_map_inf *multiboot_next_mmap();

/** Multiboot 1 functions */
int init_multiboot(multiboot_api_t *api) {
	multiboot_install_api_fns(api);
	multiboot_info *multiboot_hdr = (multiboot_info *) mboot_ptr;
	return check_multiboot_flags(multiboot_hdr->flags);
}

static int mboot_get_memory_size(void *buffer) {
	unsigned int *memsz_buffer = (unsigned int *) buffer;
	multiboot_info *mboot_inf = (multiboot_info *) mboot_ptr;

	if (!bootflag_check(mboot_inf->flags,MEMSZ_PRESENT)) {
		kinit_errno = MBOOT_ERROR_ENTRY_NOT_FOUND;
		return -1;
	}

	*memsz_buffer = (unsigned int)(mboot_inf->memorySzLo+mboot_inf->memorySzHi);
	return 0;
}

static int mboot_get_next_mmap_entry(void *buffer) {
	mmap_data *entry_buffer = (mmap_data *) buffer;
	multiboot_info *mboot_inf = (multiboot_info *) mboot_ptr;

	if (!bootflag_check(mboot_inf->flags,MMAP_PRESENT)) {
		kinit_errno = MBOOT_ERROR_ENTRY_NOT_FOUND;
		return -1;
	}

	memory_map_inf *next = multiboot_next_mmap();
	if (!next) {
		return -1;
	}

	entry_buffer->base = next->baseAddr;
	entry_buffer->length = next->length;
	entry_buffer->type = next->type;
	return 0;
}

/** internal functions */
int check_multiboot_flags(uint32_t flags) {
	#ifdef DEBUG_MULTIBOOT
	ki_printf("checking multiboot flags...\n");
	#endif
	size_t flag_entries = sizeof(multiboot_required) / sizeof(multiboot_flagcheck_t);
	for (size_t i = 0; i < flag_entries; i++) {
		if (!bootflag_check(flags, multiboot_required[i].flag)) {
			kinit_errno = multiboot_required[i].errno;
			return -1;
		}
	}
	return 0;
}

memory_map_inf *multiboot_next_mmap() {
	// grab the next memory map entry
	multiboot_info *mb_inf = (multiboot_info *) mboot_ptr;
	memory_map_inf *mb_memory_map = (memory_map_inf *) mb_inf->mmap_addr;
	unsigned int mmap_entries = (unsigned int)(mb_inf->mmap_length / sizeof (memory_map_inf));

	// make sure we are within the range of entries
	if (current_mmap_entry < mmap_entries) {
		memory_map_inf *entry = (memory_map_inf *)((unsigned int) mb_memory_map+mb_memory_map->sz+sizeof(mb_memory_map->sz)+
				(current_mmap_entry*sizeof(memory_map_inf)));

		current_mmap_entry++;
		return entry;
	}

	return NULL;
}


void multiboot_install_api_fns(multiboot_api_t *api_fns) {
	#ifdef DEBUG_MULTIBOOT
	ki_printf("installing API functions...\n");
	#endif
	api_fns->get_memory_size = mboot_get_memory_size;
	api_fns->get_next_mmap_entry = mboot_get_next_mmap_entry;
}