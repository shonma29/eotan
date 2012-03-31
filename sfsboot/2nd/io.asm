	global outb, outw, outl
	global inb, inw, inl

	section .text
	bits 32
	
outb:
	push	ebp		;ebp+8  = adress 
	mov	ebp, esp	;ebp+12 = data 
	mov	edx, [ebp + 8]
	mov	eax, [ebp + 12]
	out	dx, al
	leave
	ret

outw:
	push	ebp
	mov	ebp, esp
	mov	edx, [ebp + 8]
	mov	eax, [ebp + 12]
	out	dx, ax
	leave
	ret

outl:
	push	ebp
	mov	ebp, esp
	mov	edx, [ebp + 8]
	mov	eax, [ebp + 12]
	out	dx, eax
	leave
	ret
 
inb:	
	push	ebp		;ebp+8 = adress 
	mov	ebp, esp	;eax = return value 
	mov	edx, [ebp + 8]
	xor	eax, eax
	in	al, dx
	leave
	ret

inw:
	push	ebp
	mov	ebp, esp
	mov	edx, [ebp + 8]
	xor	eax, eax
	in	ax, dx
	leave
	ret

inl:
	push	ebp
	mov	ebp, esp
	mov	edx, [ebp + 8]
	xor	eax, eax
	in	eax, dx
	leave
	ret

