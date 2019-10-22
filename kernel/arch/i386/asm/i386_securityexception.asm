; i386_securityexception.asm
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_securityexception]
i386_securityexception:

	pushad
	call i386_exception_set_disp

	push str_SecurityException
	push dword [int_ErrorNumber]
	push str_SecurityException

	call printf
	add esp,12

	call i386_system_stop

	popad
	iretd

[section .data]

str_SecurityException 	db "Security Exception",0
int_ErrorNumber			dd 0x1e
