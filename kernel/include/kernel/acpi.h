#ifndef _KERNEL_ACPI_H
#define _KERNEL_ACPI_H 1

#include <stddef.h>
#include <stdint.h>

#ifndef __cplusplus
#include <stdbool.h>
#endif



#define ACPI_INIT_NOT_FOUND		-1
#define ACPI_INIT_ACPI_FOUND	0x01
#define ACPI_INIT_ACPI2_FOUND	0x02

int acpi_init();
void *acpi_find_header(const char *header);

/** ACPICA API bridge functions */


#endif // _KERNEL_ACPI_H