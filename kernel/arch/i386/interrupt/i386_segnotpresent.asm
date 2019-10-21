; i386_segnotpresent.asm: Handle segment errors
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_segnotpresent]
i386_segnotpresent:

	pushad

	call i386_exception_set_disp

	push str_SegmentNotPresent
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	call i386_system_stop

	popad
	iretd

[section .data]

str_SegmentNotPresent 	db "Invalid segment / Segment not present",0
int_ExceptionNumber		dd 0x0b
