[global _syscall]
[extern printf]
_syscall:

    push ebx
    call printf
    add esp,4

    iretd