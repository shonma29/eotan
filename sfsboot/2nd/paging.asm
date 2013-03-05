PAGE_DIR_ADDR	equ 2000h
PAGE_FLAG	equ 80000000h

	section .text
	bits 32

	global enable_page, get_cr3


;-----------------------------------------------------------

enable_page:
	mov	eax, PAGE_DIR_ADDR
	mov	cr3, eax
	mov	eax, cr0
	or	eax, PAGE_FLAG 
	mov	cr0, eax
	jmp	short pflush
pflush:
	ret

;-----------------------------------------------------------
get_cr3:
	mov	eax, cr3
	ret
