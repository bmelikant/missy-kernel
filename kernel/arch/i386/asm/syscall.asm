[global _syscall]
[extern printf]
_syscall:

    push testline
    call printf
    add esp,8

hang:

    jmp hang

testline db "Hello, world from userspace!",13,10,0