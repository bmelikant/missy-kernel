OUTPUT_FORMAT("elf32-i386")
ENTRY(kstart)

SECTIONS {

	. = 0x0007e000;

	.initasm : AT(ADDR(.initasm)) {
		*(.bootstrap)
	}

    . = 0x00100000;
    kernel_start = .;

    .startup : ALIGN(4K) {
        *(.multiboot)
        src/init/kinit.o
        src/init/kutils.o
        src/init/multiboot/api.o
		src/init/multiboot/mboot.o
		src/init/multiboot/mboot2.o
        src/init/i386/memory/kmemlow.o
		src/init/i386/memory/paging.o
        src/init/i386/kterm.o
    }

    . += 0xC0000000;

    .text : AT(ADDR(.text) - 0xC0000000) {
		*(.paging_jump)
        *(.text)
    }

    .data : AT(ADDR(.data) - 0xC0000000) {
        *(.data)
        *(.rodata)
        *(.kernel_errno)
    }

    .bss : AT(ADDR(.bss) - 0xC0000000) {
        *(.bss)
        *(.common)
    }

    kernel_end = .;
    kernel_end_phys = . - 0xC0000000;

    .kernelend ALIGN(4K) : AT(ADDR(.kernelend) - 0xC0000000) {
        *(.kernelend)
    }
}