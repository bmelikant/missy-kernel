; i386_genprotectfault.asm - Handle general protection fault
; Ben Melikant 11/2/16

%include "kernel/exception.inc"

%define GPF_ERROR_SELECTOR_IDT	0x02
%define GPF_ERROR_SELECTOR_LDT	0x04

[global i386_genprotectfault]
i386_genprotectfault:

	; get error code then save registers
	pop dword [int_gpfCode]
	pushad

	call i386_exception_set_disp
	
	push str_GeneralProtectionFault
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	; see if a descriptor caused the code
	mov eax,dword [int_gpfCode]
	or eax,eax
	jz .other_error

	; a descriptor caused the code. see what kind of descriptor
	mov eax,dword [int_gpfCode]
	and eax,GPF_ERROR_SELECTOR_IDT
	jnz .idt_entry

	and eax,GPF_ERROR_SELECTOR_LDT
	jnz .ldt_entry

	; ok, it was a global descriptor entry
	push str_GPFGDTError
	call puts
	add esp,4

	jmp .finish

.idt_entry:

	push str_GPFIDTError
	call puts
	add esp,4

	jmp .finish

.ldt_entry:

	push str_GPFLDTError
	call puts
	add esp,4

	jmp .finish

.other_error

	push str_GPFOther
	call puts
	add esp,4

.finish:

	; print the error code
	push dword [int_gpfCode]
	push str_GPFErrorCodeStr
	call printf
	add esp,8

	call i386_system_stop

	popad
	iretd

[section .data]

str_GeneralProtectionFault 	db "General Protection Fault",0
str_GPFOther				db "A non-segment error caused this fault",0
str_GPFGDTError				db "A segment fault in the GDT caused this fault",0
str_GPFIDTError				db "A segment fault in the IDT caused this fault",0
str_GPFLDTError				db "A segment fault in the LDT caused this fault",0
str_GPFErrorCodeStr			db "Error code: %d",10,0
int_ExceptionNumber			dd 0x0d
int_gpfCode					dd 0x00
