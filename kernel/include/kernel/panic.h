#ifndef _KERNEL_PANIC_H
#define _KERNEL_PANIC_H

__attribute__((__noreturn__))
void kernel_panic(const char *cause);

#endif // _KERNEL_PANIC_H