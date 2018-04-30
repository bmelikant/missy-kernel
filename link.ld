OUTPUT_FORMAT("elf32-i386")
ENTRY(kstart)

SECTIONS {

	. = 0xC0000000;
	_KERNEL_START = .;

	.text ALIGN(4K) : AT(ADDR(.text)-0xBFF00000) {
		*(.multiboot)
		EXCLUDE_FILE (*src/balloc_stub.o *src/kernel_stub.o *src/multiboot.o *src/early_terminal.o) *(.text)
	}

	.data ALIGN(4K) : AT(ADDR(.data)-0xBFF00000) {
		EXCLUDE_FILE (*src/balloc_stub.o *src/kernel_stub.o *src/multiboot.o *src/early_terminal.o) *(.data)
		EXCLUDE_FILE (*src/balloc_stub.o *src/kernel_stub.o *src/multiboot.o *src/early_terminal.o) *(.rodata*)
	}

	.bss ALIGN(4K) : AT(ADDR(.bss)-0xBFF00000) {
		EXCLUDE_FILE (*src/balloc_stub.o *src/kernel_stub.o *src/multiboot.o *src/early_terminal.o) *(COMMON*)
		EXCLUDE_FILE (*src/balloc_stub.o *src/kernel_stub.o *src/multiboot.o *src/early_terminal.o) *(.bss*)
	}

	_KERNEL_END = .;
	_KERNEL_SIZE = _KERNEL_END - _KERNEL_START;

	. = 0x00100000 + _KERNEL_SIZE;
	_LOADER_STUB_START = .;
	
	.startup BLOCK(4K) : ALIGN(4K) {
		*(.bootstrap)
		src/multiboot.o
		src/balloc_stub.o
		src/kernel_stub.o
		src/early_terminal.o
	}

	_LOADER_STUB_END = .;
	_LOADER_STUB_SIZE = _LOADER_STUB_END - _LOADER_STUB_START;
	_KERNEL_TOTAL_SIZE = _KERNEL_SIZE + _LOADER_STUB_SIZE;

	.kheap BLOCK(4K) : ALIGN(4K) {
	}

	_KHEAP_START = ADDR(.kheap);
}