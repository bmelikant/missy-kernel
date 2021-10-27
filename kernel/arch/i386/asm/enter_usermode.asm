; enter_usermode takes a pointer into some userspace code to transfer execution
[global enter_usermode]
extern userspace_method
extern printf
string db 'value of eax is 0x%x',13,10,0

enter_usermode:

    push ebp
    mov ebp,esp

    ; test to see if method call works...
    mov eax,[esp+4]

    push dword [eax]
    push string
    call printf

    add esp,8

    pop ebp
    ret

    mov ax,(4 * 8) | 3     ; ring 3 data segment
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax

    mov eax,esp
    push (4*8) | 3
    push eax
    pushf
    push (3*8) | 3
    push edx            ; address of called method

    iret                ; move to user mode!