SEG_BOOT 	equ	07c0h 	
SEG_1STBOOT 	equ	7000h
SEG_LOADPOINT	equ	0800h
OFF_LOADPOINT	equ	0000h
SEG_2NDBOOT	equ	0800h
OFF_2NDBOOT	equ	0000h

MAX_FRAGMENT 	equ	8			;最大フラグメント数	


		section .text

		org 0000

first_boot:
		mov	ax, SEG_BOOT		;SEG_1STBOOT:0000に移動
		mov	ds, ax
		mov	ax, SEG_1STBOOT
		mov	es, ax
		mov	cx, 256
		sub	si, si
		sub	di, di
		cld
		rep
		movsw
		jmp	SEG_1STBOOT:restart

restart:
		mov	ax, SEG_1STBOOT		;セグメントレジスタとスタックポインタ
		mov	es, ax			;の初期化
		mov	ss, ax
		mov	ds, ax
		mov	sp, 2048

		mov	bx, message
		call	print

		xor	ah, ah			;ブートデバイスをリセット
		mov	dl, [bootdev]
		int	13h

load_2ndboot:
		mov	bp, map
		mov	byte [frg], MAX_FRAGMENT
		mov	word [seg_addr], SEG_LOADPOINT
		mov 	word [off_addr], OFF_LOADPOINT

load_fragment:
		mov	cx, [bp]		;連続セクター数
		test	cx, cx
		jz	l2nd_end		;終了

		mov	ax, [bp + 2]		;開始セクター
		mov	[cylinder], ax
		mov	al, [bp + 4]
		mov	[head], al
		mov	al, [bp + 5]
		mov	[sector], al

lf_loop:
		mov	al, [sector]
		cmp	al, [n_sector]   	
		jna	lf_loop1		;if(sector < n_sector) 
		mov	byte [sector], 1	;sector = 1
		inc	byte [head]		;head = head + 1
		mov	al, [head]
		cmp	al, [n_head]		
		jne	lf_loop1		;if(head == n_head) 
		xor	al, al			
		mov	[head], al		;head = 0	
		inc	word [cylinder]		;cylinder = cylinder + 1
lf_loop1:
		call	readdisk
		jc	fatal

		push	es			;print dot
		push	bx
		mov	bx, dot
		mov	ax, SEG_1STBOOT
		mov	es, ax
		call	print
		pop	bx
		pop	es
				
		inc	byte [sector]		;sector = sector + 1	
		add	word [seg_addr], 0020h	;512byte/sector

		loop	lf_loop
		add	bp, 6			;次のフラグメントを指す
lf_end:		
		dec	byte [frg]
		jnz	load_fragment

l2nd_end:
		call	stopmotor
		jmp	SEG_2NDBOOT:OFF_2NDBOOT

;-----------------------------------------------------------
fatal:
		mov	ax, SEG_1STBOOT 
		mov	es, ax
		mov	bx, err_msg
		call	print
		call	stopmotor
fatal_loop:	
		jmp	fatal_loop		

;-----------------------------------------------------------
stopmotor:
		mov	dx, 03f2h
		xor	al, al
		out	dx, al
		ret

;-----------------------------------------------------------
readdisk:
		push	ax
		push	bx
		push	cx
		push	dx
		push	bp
readdisk1:
		mov	dl, [bootdev] 
		mov	dh, [head]
		mov	ch, [cylinder]
		mov	cl, [cylinder + 1]
		ror	cl, 2
		and	cl, 0c0h
		or	cl, [sector]
		mov	ax, [seg_addr]
		mov	es, ax
		mov	bx, [off_addr]
		mov	ax, 201h		;1セクターリード
		int	0x13			
		jnc	 readdisk2
		xor	ah, ah			;ブートデバイスをリセット
		mov	dl, [bootdev]
		int	13h
		jmp	readdisk1
readdisk2:
		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret
;-----------------------------------------------------------
print:
		push	ax
		push	bx
		push	cx
		push 	dx
		push	bp

		push	bx
		mov	ah, 003h
		xor 	bh, bh
		int	10h
		pop	bx
		xor 	cx, cx
		mov	cl, [bx]
		inc	bx
		mov	bp, bx  
		mov	bx, 0007h
		mov	ax, 1301h
		int	10h

		pop	bp
		pop	dx
		pop	cx
		pop	bx
		pop	ax
		ret

;-----------------------------------------------------------
message:
	 	db	47
		db	"1stboot for btron/386(PC)", 0dh, 0ah
		db	"loading 2ndboot...", 0dh, 0ah
err_msg:
		db	26
		db  	"can't read 2ndboot program"		
dot:
		db	1
		db	'.'

;-----------------------------------------------------------
; work area
;

cylinder:
		dw	0
head:
		db	0
sector:
		db	0
seg_addr:	
		dw	0
off_addr:
		dw	0
frg:
		db	0

;-----------------------------------------------------------
; boot device parameter

		times 1c0h-($-$$) db 0 	;org 180h

bootdev:				;ブートデバイス
		db	00h		;00h=fd0, 80h=hda
n_cylinder:				;ディスクパラメーター
		dw	00h		; シリンダー数	
n_head:
		db	00h		; ヘッド数
n_sector:	
		db	00h		; セクター数

		db	00h		; ダミー
;-----------------------------------------------------------
; 2ndbootのマップ
; 連続セクター数と開始セクターが格納される。
; ８ブロックの断片化まで対応する。
;
;	word	連続セクター数 
;	word	シリンダー
;	byte	ヘッド
;	byte	セクター

map:
		times MAX_FRAGMENT*6 db 0

;-----------------------------------------------------------	
; magic number

		times 1feh-($-$$) db 0	;org 1feh

        	DW 0xAA55
	
	 
			
	