; i386_pagefault.asm - Handle page faults (they occur pretty often in these early stages!!)
; Ben Melikant, 11/2/16

%include "kernel/exception.inc"

%define PFBIT_PAGEPRESENT 	0x01
%define PFBIT_PAGERW		0x02
%define PFBIT_RING3			0x04
%define PFBIT_RESERVED		0x08
%define PFBIT_INSTRUCTION	0x10

%define PFFLAGS_MASK 0xfffff000

[global i386_pagefault]
i386_pagefault:

	pop dword [int_pfErrorCode]
	pushad

	call i386_exception_set_disp

	push str_PageFault
	push dword [int_ExceptionNumber]
	push str_SystemException

	call printf
	add esp,12

	; print the faulting address and the page base
	mov eax,cr2
	push eax
	push str_PageFaultAddr

	call printf
	add esp,8

	mov eax,cr2
	and eax,PFFLAGS_MASK
	push eax
	push str_PageFaultPage

	call printf
	add esp,8

	; check to see if the page was present

	mov eax,dword [int_pfErrorCode]
	and eax,PFBIT_PAGEPRESENT
	jnz .page_protection_error

	; the page wasn't present; notify of a non-existent page error

	push str_PageNotPresentError
	call puts
	add esp,4

	jmp .page_rw_bit

.page_protection_error:

	; the page was not allowed to be accessed for some reason
	push str_PageProtectionError
	call puts
	add esp,4

.page_rw_bit:

	; check to see whether the fault was on read or write
	mov eax,dword [int_pfErrorCode]
	and eax,PFBIT_PAGERW
	jnz .page_write

	; the error occured on a read
	push str_PageFaultOnRead
	call puts
	add esp,4

	jmp .page_ring_bit

.page_write:

	; the error occured on a write
	push str_PageFaultOnWrite
	call puts
	add esp,4

.page_ring_bit:

	; check which ring the processor was running in
	mov eax,dword [int_pfErrorCode]
	and eax,PFBIT_RING3
	jnz .page_ring3

	; the process was in ring 0
	push str_PageFaultInKernelMode
	call puts
	add esp,4

	jmp .page_reserved_bit

.page_ring3:

	; the process was in ring 3
	push str_PageFaultInUserMode
	call puts
	add esp,4

.page_reserved_bit:

	; check to see if this access was to a reserved bit
	mov eax,dword [int_pfErrorCode]
	and eax,PFBIT_RESERVED
	jz .instruction_bit

	push str_PageFaultOnReserved
	call puts
	add esp,4

.instruction_bit:

	; check to see if this occurred during an instruction fetch
	mov eax,dword [int_pfErrorCode]
	and eax,PFBIT_INSTRUCTION
	jz .finish

	push str_PageFaultInstruction
	call puts
	add esp,4

.finish:

	call i386_system_stop

	popad
	iretd

[section .data]

int_pfErrorCode 	dd 0x00
int_ExceptionNumber dd 0x0e

str_PageFault 				db "Page Fault",0
str_PageNotPresentError		db "The requested page was not present in system memory",0
str_PageProtectionError		db "Access violation occurred on page",0
str_PageFaultOnRead			db "The fault occurred during an attempt to read from the page",0
str_PageFaultOnWrite		db "The fault occurred during an attempt to write to the page",0
str_PageFaultInUserMode		db "The process was running in user mode",0
str_PageFaultInKernelMode	db "The process was running in kernel mode",0
str_PageFaultOnReserved		db "The page fault occured due to reserved field read",0
str_PageFaultInstruction	db "The fault was caused on an instruction fetch",0
str_PageFaultAddr			db "Fault Linear Address: 0x%x",10,0
str_PageFaultPage			db "Fault in Page: 0x%x",10,0
