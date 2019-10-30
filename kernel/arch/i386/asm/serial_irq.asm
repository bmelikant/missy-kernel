;------------------------------------------------------------
; _serial_irq: interrupt handler for the serial port device
;------------------------------------------------------------

[bits 32]
[section .text]

[extern _read_serial_input]
[global _serial_irq]
_serial_irq:

	pushad

	call _read_serial_input

	mov al,0x20
	out 0x20,al

	popad
	iret