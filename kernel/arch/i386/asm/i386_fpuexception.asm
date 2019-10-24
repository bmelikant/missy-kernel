; i386_fpuexception.asm - Handle exceptions with the x87 floating point unit
; can happen when the unit is not present or has not been initialized
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

; vector 0x10
[global i386_fpuexception]
i386_fpuexception:

	pushad

	push str_FPUException
	push dword [int_ErrorNumber]
	push str_SystemException

	call printf
	add esp,12

	popad
	iretd

[section .data]

str_FPUException 	db "Floating point unit exception",0
int_ErrorNumber		dd 0x10
