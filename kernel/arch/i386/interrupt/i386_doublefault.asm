; i386_doublefault.asm - handle double fault exception
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

; vector 0x08
[global i386_doublefault]
i386_doublefault:

	pushad
	
	call i386_exception_set_disp
	
	push str_DoubleFault
	push dword [int_ExceptionNumber]
	push str_SystemException
	
	call printf
	add esp,12
	
	call i386_system_stop
	
	popad
	iretd
	
[section .data]

str_DoubleFault 	db "CPU Double fault",0
int_ExceptionNumber dd 0x08
