;-----------------------------------------------------------
; start16.asm
;  このコードは、0800h:0000hにロードされる。
;  GDT,IDTを設定し、A20を有効にした後、プロテクトモードに移行し、
;  start32にジャンプする。

GDT	equ	01000h
IDT	equ	02000h

START_ADDR	equ	8000h	; startがロードされるアドレス
START32_ADDR	equ 	8f00h	; start32がロードされるアドレス。変更時は、
				;  start32.asmのorg行も同時に変更すること
MEM_SIZE	equ	0ff8h	; メモリーサイズを書き込むアドレス(offset)
				;  2nd/memory.cで読まれる
SECOND		equ	0ff1h
MINUTE		equ	0ff2h
HOUR		equ	0ff3h
DAY		equ	0ff4h
MONTH		equ	0ff5h
YEAR		equ	0ff6h
CENTURY		equ	0ff7h
	
	section .text

	org	0000h

second_boot:
	mov	ax, cs
	mov	ds, ax
	mov	es, ax
	mov	ss, ax
	mov	sp, stack

;-----------------------------------------------------------
meme801:
	mov	ax, 0e801h	; メモリサイズの取得
	int     15h
	jc	mem88
	and	eax, 0ffffh	; 1M-16M 1kbyte単位 
	shl	eax, 10
	and	ebx, 0ffffh	; 16M以上 64kbyte単位
	shl	ebx, 16
	add	eax, ebx
	mov	[MEM_SIZE], eax
	jmp	get_time

mem88:
	mov	ah, 88h
	int	15h
	shl	eax, 10		; 1M-16M 1kbyte単位
	mov	[MEM_SIZE],eax
;-----------------------------------------------------------
get_time:
	xor	ax, ax
	mov	ah, 04h
	int	1ah
	mov	[CENTURY], ch
	mov	[YEAR],cl
	mov	[MONTH],dh
	mov	[DAY],dl
	mov	ah, 02h
	int	1ah
	mov	[HOUR],ch
	mov	[MINUTE],cl
	mov	[SECOND],dh
;-----------------------------------------------------------
set_gdt:
	xor	ax, ax		; GDT(1024byte)を0でクリア
	mov	es, ax		
	mov	di, GDT
	mov	cx, 512
	cld
	rep
	stosw
	mov	di, GDT		; gdt_tblをGDTにコピー
	mov	si, gdt_tbl
	mov	cx, 6*8
	rep
	movsb

	cli
	lgdt	[gdt_ptr]	; GDTRをセット
	lidt	[idt_ptr]	; IDTRをセット

;-----------------------------------------------------------
	call	empty_8042		; A20を有効にする
	mov	al, 0d1h		; linuxを参考に変更
	out	64h, al

	call	empty_8042	
	mov	al, 0dfh
	out	60h, al

	call	empty_8042

	xor     ax, ax                  ;本当にA20が有効になったか調べる
        mov     fs, ax                  ;fs = 0000h 
        dec     ax
        mov     gs, ax                  ;gs = 0ffffh
a20_wait:
        inc     ax              
        mov     [fs:0200h], ax          ;0000h:0200h ->000200h  
        cmp     ax, [gs:0210h]          ;0ffffh:0210h->100200h 
        je      a20_wait

;-----------------------------------------------------------
	cli

	mov	eax, cr0	; プロテクトモードに移行
	or	eax, 0001h
	mov	cr0, eax

	jmp	dword 08h:START32_ADDR	; csの設定,start32へ

;-----------------------------------------------------------
empty_8042:
	call 	delay
	in	al, 64h
	test	al, 01h
	jz	no_output
	call	delay
	in	al, 60h
	jmp	empty_8042
no_output:
	test	al, 02h
	jnz	empty_8042
	ret

delay:
	jmp	short $ + 2
	ret

;-----------------------------------------------------------

	align	4
gdt_ptr:
	dw	256 * 8 - 1
	dw	GDT
	dw	0

	align	4
idt_ptr:
	dw	256 * 8 - 1
	dw	IDT
	dw	0

gdt_tbl:
	db	000h, 000h, 000h, 000h, 000h, 000h, 0c0h, 000h ;00h
	db	0ffh, 0ffh, 000h, 000h, 000h, 09ah, 0cfh, 000h ;08h
	db	0ffh, 0ffh, 000h, 000h, 000h, 092h, 0cfh, 000h ;10h
	db	0f0h, 0ffh, 000h, 000h, 000h, 0fah, 0c7h, 000h ;18h
	db	0f0h, 0ffh, 000h, 000h, 000h, 0f2h, 0c7h, 000h ;20h
	db	0ffh, 0ffh, 000h, 000h, 000h, 0f6h, 0c3h, 000h ;28h

;-----------------------------------------------------------
	align	2
	times	256 db 00	; スタック領域は少ないので,call,push等
stack:				; は控え目に

;-----------------------------------------------------------

	times	(START32_ADDR-START_ADDR)-($-$$) nop	

	; start32までをnopで埋めている
	; この後に続くstart32のアドレスを調整している
