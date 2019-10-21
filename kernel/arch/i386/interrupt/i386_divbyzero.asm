; i386_divbyzero.asm - Processor division by zero exception handling routine
; Notify the user / programmer if an attempt is made to divide by zero
; Ben Melikant, 11/2/16

; standard inclusion for system exceptions
%include "kernel/exception.inc"

; vector 0x00
[global i386_divbyzero]
i386_divbyzero:

	; save all registers
	pushad

	; push the exception number and message onto the stack
	push str_DivisionByZero
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12					; clean three 4-byte arguments off the stack

	; restore registers and return to caller
	popad
	iretd

; exception data - always place in .data section!
[section .data]

str_DivisionByZero 	db "Division by zero",0
int_ExceptionNumber dd 0x0
