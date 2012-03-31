;-----------------------------------------------------------
;start32
; 8f00h�˥��ɤ���롣����������ϡ�32bit�ץ�ƥ��ȥ⡼�ɤˤʤäƤ��롣  
; �ƥ��쥯�����ͤ����ꤷ��9000h�˥��ɤ���Ƥ���,mode32��
; MODE32_ADDR�˥��ԡ������¹Ԥ�_main�˰ܤ���

MODE32_ADDR	equ 380000h
MODE32_SIZE	equ 8ffch

	section .text
	bits 32

	org  08f00h			; start16.asm�⻲�ȤΤ���	

start32:
	mov	eax, 010h		; ���쥯���ͤ�����
	mov	ds, eax
	mov	es, eax
	mov	fs, eax
	mov	gs, eax
	lss	esp, [stack_ptr] 	; ss,sp������

	mov	esi, 9000h		; mode32��MODE32_ADDR�˰�ư
	mov	edi, MODE32_ADDR	;
	mov	ecx, [MODE32_SIZE]	; 
	rep				; 
	movsb

	xor	eax, eax
	push	eax
	push	eax
	push	eax
	cld
	call	08h:MODE32_ADDR

;-----------------------------------------------------------
	align	4

stack_ptr:				; MODE32_ADDR���ѹ��������
	dd	0400000h
	dw	10h
 



					