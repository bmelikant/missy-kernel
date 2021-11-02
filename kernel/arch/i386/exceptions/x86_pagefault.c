#ifndef __build_i386
#error "x86 exception handlers can be built only for i386-based targets"
#endif

#include <stdio.h>

__attribute__((naked)) void
x86_pagefault() {
    int errCode = 0;
    __asm__(
        "pop (%0)\n\t"
        "pushad\n\t"
        : "m="(errCode);
    );
}

x86_pagefault_handler(int errorCode) {
    
}