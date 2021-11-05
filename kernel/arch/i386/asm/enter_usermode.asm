; enter_usermode takes a pointer into some userspace code to transfer execution
global enter_usermode
extern set_kernel_stack
enter_usermode:

    cli

    ; preserve kernel stack in TSS
    mov eax,esp
    push eax
    call set_kernel_stack
    add esp,4

    mov ax,0x23     ; ring 3 data segment
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax

    push 0x23
    mov eax,esp
    push eax
    pushfd

    ; interrupts will be enabled in user mode
    pop eax
    or eax,0x200
    push eax

    push 0x1b
    mov eax,userspace
    push eax            ; address of called method

    iretd                ; move to user mode!

userspace:

    add esp,4
    ret