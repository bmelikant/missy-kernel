; i386_machinecheck.asm - Machine state check exception handler
; Occurs if the processor supports internal checking, the mode is enabled
; and an error is found by the processor
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_machinecheck]
i386_machinecheck:

	pushad
	call i386_exception_set_disp

	push str_MachineCheck
	push dword [int_ErrorNumber]
	push str_SystemException

	call printf
	add esp,12

	call i386_system_stop

	popad
	iretd

[section .data]

str_MachineCheck 	db "Machine Check",0
int_ErrorNumber		dd 0x12
