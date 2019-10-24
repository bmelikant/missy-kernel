; i386_invalidtss.asm - Bad TSS descriptor detected
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_invalidtss]
i386_invalidtss:

	pushad

	call i386_exception_set_disp

	push str_InvalidTSS
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	call i386_system_stop

	popad
	iretd

[section .data]

str_InvalidTSS 		db "Invalid Task State Segment",0
int_ExceptionNumber dd 0x0a
