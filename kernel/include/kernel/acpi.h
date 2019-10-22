#ifndef _KERNEL_ACPI_H
#define _KERNEL_ACPI_H 1

#include <stddef.h>
#include <stdint.h>

typedef struct ACPI_SDT_HEADER {
	char signature[4];
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char oem_id[8];
	char oem_table_id[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;
}__attribute__((packed)) _acpi_sdt_header_t;

int acpi_init(_acpi_sdt_header_t *acpi_header);
void *acpi_find_header(const char *header);

#endif // _KERNEL_ACPI_H