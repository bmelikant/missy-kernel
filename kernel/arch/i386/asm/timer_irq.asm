;----------------------------------------------------------------
;
; pit_irq_handler.asm: Handle IRQs on IRQ0 (int 0x20) for
; 8253-compatible programmable interval timer
;
;----------------------------------------------------------------

[bits 32]
[section .text]

[extern pit_tick_count]
[extern pit_8254_process_callbacks]
[global timer_irq]
timer_irq:

	pushad

	call pit_8254_process_callbacks

	; send EOI command
	mov al,0x20
	out 0x20,al

	popad
	iretd
