; i386_nmi.asm - Non-maskable interrupt exception handler
; Prints message and halts the system on memory error issue
; If a watchdog timer elapsed, we will simply continue!
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

%define SYSPORT_WATCHDOG_TIMER_STATUS 	0x10
%define SYSPORT_CHANNEL_CHECK 			0x40
%define SYSPORT_PARITY_CHECK			0x80

[global i386_nmi]
i386_nmi:

	pushad

	call i386_exception_set_disp

	push str_NonMaskable
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

; check system control ports

	; first check for watchdog timer elapsed
	in al,0x92
	and al,SYSPORT_WATCHDOG_TIMER_STATUS
	jz .check_sysport_b

.watchdog_timer:

	push str_WatchdogTimer
	call puts
	add esp,4

	jmp .system_halt

.check_sysport_b:

	in al,0x61
	and al,SYSPORT_CHANNEL_CHECK
	jz .channel_err

	and al,SYSPORT_PARITY_CHECK
	jz .parity_err

	push str_UnknownError
	call puts
	add esp,4

	jmp .system_halt

.channel_err:

	push str_ChannelError
	call puts
	add esp,4

	jmp .system_halt

.parity_err:

	push str_ParityError
	call puts
	add esp,4

.system_halt:

	call i386_system_stop

	popad
	iret

[section .data]

str_NonMaskable 	db "Non-maskable interrupt",0
str_WatchdogTimer 	db "Kernel watchdog timer elapsed!",0
str_ChannelError	db "Memory channel error (errant device?)",0
str_ParityError		db "Parity error (memory failure?)",0
str_UnknownError	db "Unknown NMI error...",0
int_ExceptionNumber dd 0x02
