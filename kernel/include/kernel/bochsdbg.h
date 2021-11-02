#ifndef _BOCHS_DBG_H
#define _BOCHS_DBG_H

#ifndef __build_i386
#error "Cannot use Bochs magic breakpoint on non-i386 targets"
#endif

#define BOCHS_MAGIC_BREAKPOINT()    __asm__("xchgw %bx,%bx")

#endif