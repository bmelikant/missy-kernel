; i386_stacksegfault: Handle stack segment fault
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_stacksegfault]
i386_stacksegfault:

	pushad

	call i386_exception_set_disp

	push str_StackSegmentFault
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	call i386_system_stop

	popad
	iretd

[section .data]

str_StackSegmentFault 	db "Stack Segment Error",0
int_ExceptionNumber 	dd 0x0c
