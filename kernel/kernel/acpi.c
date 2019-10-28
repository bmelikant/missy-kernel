/**
 * ACPI reading functionality
 */

#include <kernel/acpi.h>
#include <kernel/memory.h>

#include <stdio.h>
#include <string.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

#define BDA_SEGMENT_LOCATION	0x40E		// this location holds the EBDA memory location
#define BDA_MAX_SEEK			1024		// and the header will be located somewhere in the first 1024 bytes

#define EDBA_BASE_ADDRESS		0xA0000
#define EDBA_END_ADDRESS		0x100000
#define EDBA_SIZE				(EDBA_END_ADDRESS-EDBA_BASE_ADDRESS)

#define ACPI_HEADER_ALIGN		0x10

#define ACPI_HEADER_RSDP	"RSD PTR "
#define ACPI_HEADER_RSDT	"RSDT"
#define ACPI_HEADER_FADT	"FADT"

#define ALIGN_BLOCK_UP(x)	((x &~ 0xfff)+PAGE_SIZE)
#define ALIGN_BLOCK_DOWN(x)	(x &~ 0xfff)
#define PAGE_OFFSET(x)		(x & 0xfff)

typedef struct ACPI_SDT_HEADER {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[6];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
}__attribute__((packed)) _acpi_sdt_header_t;

typedef struct ACPI_RSDP_HEADER {
	char signature[8];
	uint8_t checksum;
	char oem_id[6];
	uint8_t revision;
	uint32_t rsdt_address;
} _acpi_rsdp_header_t;

typedef struct ACPI2_RSDP_HEADER {
	_acpi_rsdp_header_t original_rsdp;
	uint32_t total_length;
	uint64_t xsdt_address;
	uint8_t checksum_extended;
	uint8_t reserved[3];
} _acpi2_rsdp_header_t;

typedef struct ACPI_RSDT_HEADER {
	_acpi_sdt_header_t acpi_header;
	uint32_t headers[];
}__attribute__((packed))_acpi_rsdt_header_t;

typedef struct ACPI_XSDT_HEADER {
	_acpi_sdt_header_t acpi_header;
	uint64_t headers[];
}__attribute__((packed)) _acpi_xsdt_header_t;

typedef _acpi_xsdt_header_t *	_acpi_xsdt_header_p;
typedef _acpi_rsdt_header_t *	_acpi_rsdt_header_p;
typedef _acpi_sdt_header_t *	_acpi_sdt_header_p;

static _acpi2_rsdp_header_t *rsdp;
static _acpi_sdt_header_t *rsdt_virtual = NULL;

bool is_acpi2 = false;

/** internal functions */
//static void *acpi_find_header(const char *header_name);
static void *acpi2_find_header(const char *header_name);


/**
 * try to locate the ACPI header
 * *** NOTE: For some reason, reading the low BDA makes the EDBA search succeed
 * on qemu and bochs? ***
 */
int acpi_init() {
	char *bda_location = (char *)(*(uint16_t *) BDA_SEGMENT_LOCATION);

	// scan for the ACPI RSDP. Start in EBDA (this doesn't have to be mapped)
	printf("looking in low EBDA\n");
	for (size_t i = 0; i < BDA_MAX_SEEK; i += ACPI_HEADER_ALIGN) {
		char *acpi_string = (char *)(bda_location+i);
		if (strncmp(acpi_string,ACPI_HEADER_RSDP,strlen(ACPI_HEADER_RSDP)) == 0) {
			rsdp = (_acpi2_rsdp_header_t *)(acpi_string);
			printf("ACPI header found in lower BDA\n");
			if (rsdp->original_rsdp.revision > 0) {
				is_acpi2 = true;
				return ACPI_INIT_ACPI2_FOUND;
			}
			return ACPI_INIT_ACPI_FOUND;
		}
	}

	// scan the upper EDBA range
	printf("looking in upper EBDA\n");
	for (size_t i = 0; i < EDBA_SIZE; i += ACPI_HEADER_ALIGN) {
		char *acpi_string = (char *)(EDBA_BASE_ADDRESS+i);
		if (strncmp(ACPI_HEADER_RSDP,acpi_string,strlen(ACPI_HEADER_RSDP)) == 0) {
			rsdp = (_acpi2_rsdp_header_t *)(acpi_string);
			printf("ACPI header found in upper EDBA\n");
			if (rsdp->original_rsdp.revision > 0) {
				is_acpi2 = true;
				return ACPI_INIT_ACPI2_FOUND;
			}
			return ACPI_INIT_ACPI_FOUND;
		}
	}

	printf("Could not find ACPI header\n");
	return ACPI_INIT_NOT_FOUND;
}



void *acpi_find_header(const char *header_name) {
	if (is_acpi2) {
		return acpi2_find_header(header_name);
	} else {
		void *rsdt_virtual = memory_request_temp_map((void *) rsdp->original_rsdp.rsdt_address);
		printf("rsdt address: 0x%x\n", (uint32_t)(rsdp->original_rsdp.rsdt_address));
		if (rsdt_virtual) {
			_acpi_rsdt_header_p rsdt_header = (_acpi_rsdt_header_p) rsdt_virtual;
			_acpi_sdt_header_p _header = NULL;
			void *header_physical = NULL;

			size_t entries = (size_t)((rsdt_header->acpi_header.length - sizeof(_acpi_sdt_header_t)) / sizeof(uint32_t));
			printf("size of header: %u\n", rsdt_header->acpi_header.length);
			printf("number of entries: %u\n", (uint32_t) entries);
			
			for (size_t i = 0; i < entries; i++) {
				printf("address of next header: 0x%x\n", rsdt_header->headers[i]);
				uintptr_t header_page = ALIGN_BLOCK_DOWN(rsdt_header->headers[i]);
				uintptr_t rsdt_page = ALIGN_BLOCK_DOWN(rsdp->original_rsdp.rsdt_address);

				printf("new header page: 0x%x, rsdt page: 0x%x\n", header_page, rsdt_page);
				bool mapped_new = false;

				// check to see if the new header is within the previous mapped page
				if (header_page == rsdt_page) {
					_header = (_acpi_sdt_header_p)(ALIGN_BLOCK_DOWN((uintptr_t) rsdt_virtual)+PAGE_OFFSET(rsdt_header->headers[i]));
				} else {
					_header = memory_request_temp_map((void *) rsdt_header->headers[i]);
					mapped_new = true;
				}

				printf("header virtual address: 0x%x\n", (uint32_t) _header);
				bool found_header = false;
				if (strncmp(_header->signature,header_name,strlen(header_name)) == 0) {
					printf("found header %s\n", header_name);
					found_header = true;
				}
				if (mapped_new) {
					memory_release_temp_map(_header);
				}
				if (found_header) {
					header_physical = (void *)(rsdt_header->headers[i]);
					break;
				}
			}
			
			memory_release_temp_map(rsdt_virtual);
			return header_physical;
		} else {
			printf("could not map temporary page for rsdt\n");
			return NULL;
		}
	}
}

void *acpi2_find_header(const char *header_name) {
	void *xsdt_virtual = memory_request_temp_map((void *) rsdp->xsdt_address);
	if (xsdt_virtual) {
		_acpi_xsdt_header_p xsdt_header = (_acpi_xsdt_header_p) xsdt_virtual;
		size_t entries = (size_t)((xsdt_header->acpi_header.length - sizeof(_acpi_sdt_header_t)) / sizeof(uint64_t));

		for (size_t i = 0; i < entries; i++) {
			printf("address of next header: %u\n", (uint32_t)(xsdt_header->headers[i]));
		}
		memory_release_temp_map(xsdt_virtual);
		return NULL;
	}

	printf("couldn't map temporary page for xsdt\n");
	return NULL;
}