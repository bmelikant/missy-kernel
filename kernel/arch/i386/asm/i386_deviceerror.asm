; i386_deviceerror.asm - Handle absent FPU
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

; vector 0x07
[global i386_deviceerror]
i386_deviceerror:

	pushad

	call i386_exception_set_disp

	push str_DeviceError
	push int_ExceptionNumber
	push str_SystemException

	call printf
	add esp,12

	call i386_system_stop

	popad
	iretd

[section .data]

str_DeviceError 	db "Floating-point instruction with no FPU present",0
int_ExceptionNumber dd 0x07
