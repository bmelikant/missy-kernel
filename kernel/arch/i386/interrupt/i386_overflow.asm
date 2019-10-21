; i386_overflow.asm: Handle data overflows
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_overflow]
i386_overflow:

	pushad
	call i386_exception_set_disp

	push str_Overflow
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	call i386_system_stop

	; should never get called!
	popad
	iretd

[section .data]

str_Overflow 		db "Overflow exception",0
int_ExceptionNumber dd 0x04
