; i386_virtualizeexception: Virtualization exception (No idea...)
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

[global i386_virtualizeexception]
i386_virtualizeexception:

	pushad

	push str_VirtualizationException
	push dword [int_ErrorNumber]
	push str_SystemException

	call printf
	add esp,12

	popad
	iretd

[section .data]

str_VirtualizationException db "Virtualization Exception",0
int_ErrorNumber				dd 0x14
