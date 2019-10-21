;-----------------------------------------
; default.asm: default interrupt handler
;-----------------------------------------
[section .text]
align 4

[global default_interrupt_vector]
[extern puts]
default_interrupt_vector:

	pushad

	push dword msg_default_handler
	call puts
	add esp,4

	popad
	iretd

[section .data]
msg_default_handler db 'Default interrupt handler was called',0