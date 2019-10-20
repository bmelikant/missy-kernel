OUTPUT_FORMAT("elf32-i386")
ENTRY(kstart)

SECTIONS {

	. = 0x0007e000;

	.initasm : AT(ADDR(.initasm)) {
		*(.multiboot)
		*(.bootstrap)
	}

    . = 0x00100000;
    kernel_start = .;

    .startup : ALIGN(4K) {
        ../init/init/kinit.o
        ../init/init/kutils.o
        ../init/init/multiboot/api.o
		../init/init/multiboot/mboot.o
		../init/init/multiboot/mboot2.o
        ../init/init/memory/kmemlow.o
		../init/init/memory/paging.o
        ../init/init/kterm.o
    }

    . += 0xC0000000;

    .text : AT(ADDR(.text) - 0xC0000000) {
		*(.paging_jump)
        *(.text .text.*)
    }

    .data : AT(ADDR(.data) - 0xC0000000) {
        *(.data .data.*)
        *(.rodata .rodata.*)
        *(.kernel_errno)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000) {
        *(.bss .bss.*)
        *(.common .common.*)
    }

    kernel_end = .;
    kernel_end_phys = . - 0xC0000000;

    .kernelend ALIGN(4K) : AT(ADDR(.kernelend) - 0xC0000000) {
        *(.kernelend)
    }
}