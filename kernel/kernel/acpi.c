/**
 * ACPI reading functionality
 */

#include <kernel/acpi.h>
#include <kernel/memory.h>

#include <string.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef struct ACPI_RSDT_HEADER {
	_acpi_sdt_header_t acpi_header;
	uint32_t *headers;
}__attribute__((packed))_acpi_rsdt_header_t;

typedef struct ACPI_XSDT_HEADER {
	_acpi_sdt_header_t acpi_header;
	uint64_t *headers;
}__attribute__((packed)) _acpi_xsdt_header_t;

_acpi_sdt_header_t *rsdt_ptr;
static _acpi_sdt_header_t *rsdt_virtual = NULL;

bool is_xsdt = false;

int acpi_init(_acpi_sdt_header_t *rsdt) {
	// temporarily map the acpi table into 0xffbff000
	rsdt_virtual = (_acpi_sdt_header_t *) memory_request_temp_map(rsdt);
	printf("RSDT virtual address (temp mapping): 0x%x\n", (uint32_t) rsdt_virtual);

	if (!rsdt_virtual) {
		return -1;
	}

	if (strncmp((char *)(rsdt_virtual),"XSDT",strlen("XSDT")) == 0) {
		is_xsdt = true;
	}

	return 0;
}

void *acpi_find_header(const char *header_name) {

}