#include <version.h>

#include <init/kterm.h>
#include <init/kutils.h>
#include <init/memory/kmemory.h>
#include <init/multiboot/multiboot.h>
#include <init/kerrno.h>

void early_welcome_message() {
    kernel_early_printf("missy kernel %s\n", KERNEL_VERSION_STRING);
#ifdef DEBUG_BUILD
    kernel_early_printf("this is a debug build. debugging output will be shown\n");
#endif
}



void kernel_early_init(void *mboot_hdr, unsigned int magic) {
    early_welcome_message();

    int result = multiboot_init(mboot_hdr, magic);
    if (result != 0) {
        kernel_early_printf("[PANIC]: Multiboot data format could not be determined: %d\n", kinit_errno);
        early_panic();
    }

    kmemory_init_allocator();
}