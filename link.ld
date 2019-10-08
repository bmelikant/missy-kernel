OUTPUT_FORMAT("elf32-i386")
ENTRY(kstart)

SECTIONS {

    . = 0x00100000;

    .startup : ALIGN(4K) {
        *(.multiboot)
        *(.bootstrap)
        src/init/kinit.o
        src/init/kutils.o
        src/init/kterm.o
        src/init/multiboot/multiboot.o
        src/init/memory/kmemory.o
    }

    . += 0xC0000000;

    .text ALIGN(4K) : AT(ADDR(.text) - 0xC0000000) {
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