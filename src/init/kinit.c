#include <init/kterm.h>
#include <init/memory/kmemory.h>

void kernel_early_init(void *mboot_hdr, unsigned int magic) {
    kernel_early_printf("Test string. Hello, world!\n");
    kmemory_init_allocator();
}