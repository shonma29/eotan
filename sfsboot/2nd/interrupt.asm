%macro	int0_16_handler 1	; 例外処理用
	cli
	pushad
	push	byte %1			
	call	fault
	pop	eax
	popad
	iret		
%endmacro	
	
%macro	int8259master_handler 1	; ハードウェア割り込み 1-8用 
	cli
	pushad
	push	byte %1
	call	interrupt
	pop	eax
	cli
	mov	al, 20h		; set EOI
	out	20h, al		; master
	popad
	sti
	iret
%endmacro

%macro	int8259slave_handler 1	; ハードウェア割り込み 9-16用
	cli
	pushad
	push	byte %1
	call	interrupt
	pop	eax
	cli
	mov	al, 20h		; set EOI
	out	0a0h, al	; slave
	out	20h, al		; master
	popad
	sti
	iret
%endmacro	

	
	global	open_interrupt
	global	ignore_handler
	global	int32_handler, int33_handler,                int35_handler
	global	int36_handler, int37_handler, int38_handler, int39_handler
	global	int40_handler, int41_handler, int42_handler, int43_handler
	global	int44_handler, int45_handler ,int46_handler, int47_handler

	extern 	fault, interrupt, boot_printf
	
;-----------------------------------------------------------
	
	section .text
	bits 32
	
	
intr_msg:
	db	"ignore int.\n"

ignore_handler:
	cli
	pushad
	push	long intr_msg
	call	boot_printf
	pop	eax
	popad
	sti
	iret

int0_handler:
	int0_16_handler 0
	
int1_handler:
	int0_16_handler 1
	
int2_handler:
	int0_16_handler 2
	
int3_handler:
	int0_16_handler 3
	
int4_handler:
	int0_16_handler 4
	
int5_handler:
	int0_16_handler 5

int6_handler:
	int0_16_handler 6
	
int7_handler:
	int0_16_handler 7
	
int8_handler:
	int0_16_handler 8
	
int9_handler:
	int0_16_handler 9
		
int10_handler:
	int0_16_handler 10
	
int11_handler:
	int0_16_handler 11
	
int12_handler:
	int0_16_handler 12

int13_handler:
	int0_16_handler 13
		
int14_handler:
	int0_16_handler 14
	
int15_handler:
	int0_16_handler 15

int16_handler:
	int0_16_handler 16

int32_handler:				
	int8259master_handler 32
	
int33_handler:
	int8259master_handler 33

int35_handler:
	int8259master_handler 35

int36_handler:
	int8259master_handler 36
	
int37_handler:
	int8259master_handler 37

int38_handler:
	int8259master_handler 38

int39_handler:
	int8259master_handler 39	

int40_handler:
	int8259slave_handler 40
	
int41_handler:
	int8259slave_handler 41
	
int42_handler:
	int8259slave_handler 42

int43_handler:
	int8259slave_handler 43
	
int44_handler:
	int8259slave_handler 44
	
int45_handler:
	int8259slave_handler 45
				
int46_handler:
	int8259slave_handler 46	

int47_handler:
	int8259slave_handler 47

