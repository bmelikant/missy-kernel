; i386_bounderror.asm - Boundary exception handler
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

; vector 0x05
[global i386_bounderror]
i386_bounderror:

	pushad

	push str_BoundaryError
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	popad
	iretd

[section .data]

str_BoundaryError 	db "Array index out of bounds",0
int_ExceptionNumber dd 0x05
