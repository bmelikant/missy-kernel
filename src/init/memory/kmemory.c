#include <init/memory/kmemory.h>
#include <init/kterm.h>

extern unsigned int _mbitmap;
unsigned int *_mbitmap_physical;

int kmemory_init_allocator() {
    // locate the bitmap in physical memory
    unsigned int address = (unsigned int) &_mbitmap - 0xc0000000;
    _mbitmap_physical = (unsigned int *) address;

    
}