bits 32

; multiboot information definitions

MULTIBOOT_PAGE_ALIGN   equ 1<<0
MULTIBOOT_MEMORY_INFO  equ 1<<1
MULTIBOOT_HEADER_MAGIC equ 0x1badb002
MULTIBOOT_HEADER_FLAGS equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
MULTIBOOT_CHECKSUM     equ -(MULTIBOOT_HEADER_MAGIC+MULTIBOOT_HEADER_FLAGS)

MULTIBOOT2_MAGIC        equ 0xe85250d6
MULTIBOOT2_ARCH         equ 0
MULTIBOOT2_HEADER_SZ    equ multiboot2_end-multiboot2_hdr
MULTIBOOT2_CHECKSUM     equ -(MULTIBOOT2_MAGIC+MULTIBOOT2_ARCH+MULTIBOOT2_HEADER_SZ)

; place the multiboot header in it's own section

[section .multiboot]
align 4
multiboot2_hdr:

    dd MULTIBOOT2_MAGIC         ; magic value
    dd MULTIBOOT2_ARCH          ; architecture type
    dd MULTIBOOT2_HEADER_SZ     ; header size
    dd MULTIBOOT2_CHECKSUM      ; checksum

    ; end tag
    dw 0        ; tag type 0
    dw 0        ; tag flags none
    dd 8        ; tag size 8 bytes

multiboot2_end:

multiboot_hdr:

	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd MULTIBOOT_CHECKSUM

[section .bootstrap]
align 4

[extern kernel_early_init]
[extern _page_directory]
[extern _init]
[global early_panic]
[global kstart]
[global kinit_errno]

gdt_start:

	dd 0,0
	db 0xff,0xff,0x00,0x00,0x00,10011010b,11001111b,0x00
	db 0xff,0xff,0x00,0x00,0x00,10010010b,11001111b,0x00

gdt_table:

	dw gdt_table-gdt_start-1
	dd gdt_start

kinit_errno dd 0x0
_kernel_params:
	kernel_stack 	dd 0
	kernel_heap		dd 0
	memory_bitmap 	dd 0
	total_blocks	dd 0
	used_blocks		dd 0
	rsdt_address	dd 0

early_panic:

	jmp $

kstart:
	
	lgdt [gdt_table]
	jmp 0x08:next

next:

	xchg eax,edx

	mov eax,0x10
	mov ax,0x10
	mov ds,ax
	mov es,ax
	mov fs,ax
	mov gs,ax
	
	mov ss,ax
	mov esp,0x90000
	
	push dword _kernel_params
	push edx
	push ebx
	call kernel_early_init

	; enter paging mode
	mov eax,dword [_page_directory]
	mov cr3,eax

	mov eax,cr0
	or eax,0x80000001
	mov cr0,eax

	xor eax,eax
	jmp _paging_start

[section .paging_jump]
[extern kernel_main]
_paging_start:

	call _init				; initialize the C runtime

	; fix the stack... ugh
	mov esp,dword [kernel_stack]
	mov ebp,esp

	push dword _kernel_params
	call kernel_main

	jmp $

[section .kernel_errno]
[global _kerrno]
_kerrno dd 0x00

[section .kernelend]
[global _mbitmap]

_mbitmap:	; this is where the page tables will be located, at the end of the kernel. This address can be adjusted to form a physical address
	dd 0