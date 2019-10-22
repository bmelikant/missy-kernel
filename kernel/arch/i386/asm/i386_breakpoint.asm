; i386_breakpoint.asm - Handle embedded breakpoints (int 3)
; This code will simply halt until debugging is added later
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

; vector 0x03
[global i386_breakpoint]
i386_breakpoint:

	pushad

	push str_Breakpoint
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	popad
	iretd

[section .data]

str_Breakpoint 		db "Breakpoint detected",0
int_ExceptionNumber dd 0x03
