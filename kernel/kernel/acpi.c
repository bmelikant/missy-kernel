/**
 * ACPI reading functionality
 */

#include <kernel/acpi.h>

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
bool is_xsdt = false;

int acpi_init(_acpi_sdt_header_t *rsdt) {
	// temporarily map the acpi table into 0xffbff000
	if (rsdt->revision > 1) {
		is_xsdt = true;
	}

}
void *acpi_find_header(const char *header_name) {

}