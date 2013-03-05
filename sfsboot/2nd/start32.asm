;-----------------------------------------------------------
;start32
; 8f00hにロードされる。ここから先は、32bitプロテクトモードになっている。  
; 各セレクター値を設定し、9000hにロードされている,mode32を
; MODE32_ADDRにコピーし、実行を_mainに移す。

MODE32_ADDR	equ 9000h
MODE32_SIZE	equ 8ffch

	section .text
	bits 32

	org  08f00h			; start16.asmも参照のこと	

start32:
	mov	eax, 010h		; セレクタ値を設定
	mov	ds, eax
	mov	es, eax
	mov	fs, eax
	mov	gs, eax
	lss	esp, [stack_ptr] 	; ss,spの設定

	xor	eax, eax
	push	eax
	push	eax
	push	eax
	cld
	call	08h:MODE32_ADDR

;-----------------------------------------------------------
	align	4

stack_ptr:				; MODE32_ADDRを変更時は注意
	dd	0800000h
	dw	10h

