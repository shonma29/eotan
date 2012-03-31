/* $Source: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/i386/interrupt.S,v $
 *
 * $Revision: 1.3 $
 * $Date: 2000/05/06 03:47:34 $
 * $Author: naniwa $
 *
 * Discription
 *
 */

#define MASTER_8259A_COM	$0x20
#define MASTER_8259A_DATA	$0x21

#define SLAVE_8259A_COM		$0xa0
#define SLAVE_8259A_DATA	$0xa1


.globl	open_interrupt, ignore_handler
.globl  int1_handler	
.globl  int2_handler	
.globl  int3_handler	
.globl  int4_handler	
.globl  int5_handler	
.globl	int6_handler
.globl  int8_handler
.globl  int9_handler
.globl	int10_handler
.globl  int11_handler
.globl  int12_handler
.globl  int13_handler
.globl	int14_handler
.globl  int15_handler
.globl  int16_handler
.globl  int17_handler
#ifdef notdef
.globl  int18_handler
.globl  int19_handler
.globl  int20_handler
.globl  int21_handler
.globl  int22_handler
.globl  int23_handler
.globl  int24_handler
.globl  int25_handler
.globl  int26_handler
.globl  int27_handler
.globl  int28_handler
.globl  int29_handler
.globl  int30_handler
.globl  int31_handler
#endif
.globl  int32_handler
.globl  int33_handler
.globl  int34_handler
.globl	int35_handler
.globl	int36_handler
.globl  int37_handler
.globl  int38_handler
.globl	int40_handler
.globl	int41_handler
.globl	int43_handler
.globl  int44_handler
.globl	int46_handler
.globl	syscall_handler, int64_handler, posix_handler
.globl  btron_handler, int66_handler	
/* .globl	int32_handler, int33_handler, int41_handler, int43_handler */
.globl	dis_int, ena_int

.text


intr_msg:	.ascii	"ignore int.\n"
ignore_handler:
		cli
		movb	0x20, %al
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
		iret

/****************************************************************************
 * int0_handler --- 除算エラー
 *
 */
int0_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$0
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret
	
/****************************************************************************
 * int1_handler --- デバッガ割り込み
 *
 */
int1_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$1
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret
	
/****************************************************************************
 * int2_handler --- MNI
 */
int2_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$2
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret

/*************************************************************************
 * int3_handler --- break point 割り込み
 *
 */
int3_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$3
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 * int4_handler --- overflow 割り込み
 *
 */
int4_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$4
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret
	
/**************************************************************************
 * int5_handler --- SCSI
 * 
 */
int5_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		pushl	$5
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	interrupt
		popl	%eax
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		popl	%es
		popl	%ds
		iret

/*	
		pushal
		pushl	$5
		call	fault
		popl	%eax
		popal
		iret
*/
/**************************************************************************
 *
 */
int6_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$6
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 *
 */
int7_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$7
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret


/**************************************************************************
 *
 */
int8_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$8
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret


/**************************************************************************
 *
 */
int9_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$9
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret


/**************************************************************************
 * int10_handler --- 不法ＴＳＳ例外
 *
 */
int10_handler:
		pushl	%ds
		pushl	%es
		pushal
		pushl	$10
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		addl	$4, %esp
		iret


/**************************************************************************
 *
 */
int11_handler:
		pushl	%ds
		pushl	%es
		pushal
		pushl	$11
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		addl	$4, %esp
		iret


/**************************************************************************
 * int12_handler --- stack fault.
 */
int12_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	protect_fault
		popal
		popl	%es
		popl	%ds
		addl	$4, %esp
		iret


/**************************************************************************
 * int14_handler --- protect fault.
 */
int13_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	protect_fault
		popal
		popl	%es
		popl	%ds
		addl	$4,%esp
		iret


/**************************************************************************
 * int14_handler --- page fault.
 */
int14_handler:
		pushl	%ds
		pushl	%es
                pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call page_fault
                popal
		popl	%es
		popl	%ds
		addl	$4,%esp
		iret

#if 0
/**************************************************************************
 *	将来のために予約されているだけ．
 */
int15_handler:
		pushal
		pushl	$15
		call	fault
		popl	%eax
		popal
		iret
#endif

/**************************************************************************
 * int16_handler:	コプロセッサー・フォルト
 */
int16_handler:
		pushl	$0
		pushl	%ds
		pushl	%es
		pushal
		pushl	$16
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		addl	$4,%esp
		iret
	
/**************************************************************************
 * int17_handler:	アライメント・チェック・フォルト
 */
int17_handler:
		pushl	$0
		pushl	%ds
		pushl	%es
		pushal
		pushl	$17
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		call	fault
		popl	%eax
		popal
		popl	%es
		popl	%ds
		addl	$4, %esp
		iret

/**************************************************************************
 *
 * int32_handler --- timer 
 *
 *
 */
int32_handler:
/*
		cli
*/
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
#if 1
		inb	MASTER_8259A_DATA, %al
		orb	$1, %al
		outb	%al, MASTER_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		sti
#endif
		pushl	$32
		call	interrupt
		popl	%eax
/*
		movb	$0x21, %al	# set EOI
		outw	%al, MASTER_8259A_COM
*/
#if 0
#if 1
		movb	$0x20, %al	# set EOI
		cli
		outb	%al, MASTER_8259A_COM
#else
		cli
		inb	MASTER_8259A_DATA, %al
		andb	$0xFE, %al
		outb	%al, MASTER_8259A_DATA
#endif
#endif
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 * int33_handler --- keyboard
 *
 */
int33_handler:
/*
		cli
*/
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
#if 1
		inb	MASTER_8259A_DATA, %al
		orb	$2, %al
		outb	%al, MASTER_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		sti
#endif
		pushl	$33
		call	interrupt
		popl	%eax
/*
		movb	$0x21, %al	# set EOI
		outw	%al, MASTER_8259A_COM
*/
#if 0
#if 1
		movb	$0x20, %al	# set EOI
		cli
		outb	%al, MASTER_8259A_COM
#else
		cli
		inb	MASTER_8259A_DATA, %al
		andb	$0xFD, %al
		outb	%al, MASTER_8259A_DATA
#endif
#endif
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 * int34_handler --- CRTV
 *
 *	no work.
 */
int34_handler:
		pushal
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		iret

/**************************************************************************
 * int35_handler --- INT0
 *
 *	no work.
 */
int35_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$35
		call	interrupt
		popl	%eax
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		popl	%es
		popl	%ds
		iret

/*
		cli
		pusha
		movb	$0x20, %al	# set EOI
		movw	$0x00, %dx
		outw	%al, %dx
		popa
		iret
*/

/**************************************************************************
 * int36_handler --- RS232C
 *
 *	no work.
 */
int36_handler:
		pushal
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		iret

/**************************************************************************
 * int37_handler --- INT1
 *
 *	no work.
 */
int37_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$37
		call	interrupt
		popl	%eax
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		popl	%es
		popl	%ds
		iret
/*	
		cli
		pusha
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		outw	%al, %dx
		popa
		iret
*/
	
/**************************************************************************
 * int38_handler --- floppy disk
 *
 *	no work.
 */
int38_handler:
/*
		cli
*/
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
#if 1
		inb	MASTER_8259A_DATA, %al
		orb	$0x40, %al
		outb	%al, MASTER_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		sti
#endif
		pushl	$38
		call	interrupt
		popl	%eax
#if 0
		movb	$0x20, %al	/* set EOI */
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
#endif
#ifdef notdef	
		movb	$0x20, %al	# set EOI
		outw	%al, $0x20
#endif
	
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 * int39_handler --- connect to slave 
 *
 *	no work.
 */
int39_handler:
		pushal
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		iret

/**************************************************************************
 * int40_handler --- Interval timer.
 *
 *	no work.
 */
int40_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$40
		call	interrupt
		popl	%eax
		movb	$0x20, %al
		movw	SLAVE_8259A_COM, %dx
		cli
		outb	%al, %dx
		movb	$0x0b, %al
		outb	%al, %dx
		inb	%dx, %al
		test	%al, %al
		jnz	int_40_l1
		movb	$0x20, %al
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
int_40_l1:
		popal
		popl	%es
		popl	%ds
		sti
		iret
	/*
		cli
		pusha
		movb	$0x20, %al	# set EOI
		movw	$0x00, %dx
		outw	%al, %dx
		popa
		iret
	*/

/**************************************************************************
 * int41_handler --- INT3(SASI/SCSI HD)
 *
 *	
 */
int41_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$41
		call	interrupt
		popl	%eax
		movb	$0x20, %al
		movw	SLAVE_8259A_COM, %dx
		cli
		outb	%al, %dx
		movb	$0x0b, %al
		outb	%al, %dx
		inb	%dx, %al
		test	%al, %al
		jnz	int_41_l1
		movb	$0x20, %al
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
int_41_l1:
		sti
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 * int42_handler --- INT42 (FD 640K)
 *
 *	no work.
 */
int42_handler:
		cli
		pushal
		/*
		 *
		 */
		movb	$0x20, %al
		movw	SLAVE_8259A_COM, %dx
		cli
		outb	%al, %dx
		movb	$0x0b, %al
		outb	%al, %dx
		inb	%dx, %al
		test	%al, %al
		jnz	int_42_l1
		movb	$0x20, %al
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
int_42_l1:
		popal
		sti
		iret

/**************************************************************************
 * int43_handler --- INT43 (FD 1M)
 *
 *
 */
int43_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		pushl	$43
		call	interrupt
		popl	%eax
		movb	$0x20, %al
		movw	SLAVE_8259A_COM, %dx
		cli
		outb	%al, %dx
		movb	$0x0b, %al
		outb	%al, %dx
		inb	%dx, %al
		test	%al, %al
		jnz	int_43_l1
		movb	$0x20, %al
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
int_43_l1:
		popal
		popl	%es
		popl	%ds
		sti
		iret

/**************************************************************************
 * int44_handler --- IRQ 12 (PSAUX)
 *
 *      no work.
 */
int44_handler:
                cli
		pushl	%ds
		pushl	%es
                pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
#if 1	/* original by Mr. Murata */
                movb    $0x20, %al      # set EOI
                movw    SLAVE_8259A_COM, %dx
                outb    %al, %dx
                movb    $0x0b, %al
                outb    %al, %dx
                inb     %dx, %al
                test    %al, %al
                jnz     int_44_l1
                movb    $0x20, %al
                movw    MASTER_8259A_COM, %dx
                outb    %al, %dx
#else
		inb	SLAVE_8259A_DATA, %al
		orb	$0x10, %al
		outb	%al, SLAVE_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		jmp	int44_out	# delay
int44_out:	outb	%al, SLAVE_8259A_COM
#endif
int_44_l1:
                sti
                pushl   $44
                call    interrupt       
                popl    %eax
                popal
		popl	%es
		popl	%ds
                iret


/**************************************************************************
 * int45_handler --- INT 6(MOUSE)
 *
 *	no work.
 */
int45_handler:
		cli
		pushal
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
		popal
		iret

/**************************************************************************
 * int46_handler --- IDE 0
 *
 *	no work.
 */
int46_handler:
		cli
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
#if 1
		inb	SLAVE_8259A_DATA, %al
		orb	$0x40, %al
		outb	%al, SLAVE_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		jmp	int46_out	# delay
int46_out:	outb	%al, SLAVE_8259A_COM
		sti
#endif
		pushl	$46
		call	interrupt
		popl	%eax
#ifdef notdef
		movb	$0x20, %al
		movw	SLAVE_8259A_COM, %dx
		cli
		outb	%al, %dx
		movb	$0x0b, %al
		outb	%al, %dx
		inb	%dx, %al
		test	%al, %al
		jnz	int_46_l1
		movb	$0x20, %al
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
int_46_l1:
		sti
#endif
		popal
		popl	%es
		popl	%ds
		iret

#ifdef notdef	
		cli
		pusha
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		outb	%al, %dx
		popa
		iret
#endif

/**************************************************************************
 * int47_handler --- IDE 1
 *
 *	no work.
 */
int47_handler:
		pushal
		movb	$0x20, %al	# set EOI
		movw	MASTER_8259A_COM, %dx
		cli
		outb	%al, %dx
		popal
		iret

/**************************************************************************
 * int64_handler --- ITRON system call.
 *
 *	システムコール割り込みの処理。
 *	そのため、他の割り込みと異なり、EOI などの処理を行わない。
 *
 *
 *	システムコール処理では次のことを「仮定」している。
 *	1) システムコール番号は、レジスタ eax に収められている。
 *	2) 同じくシステムコールへの引数の先頭番地は、レジスタ ebx に
 *	   収められている。
 *	3) システムコールからの返り値は、レジスタ eax に収められる。
 *	   (ユーザプロセスはこのことを「知って」いることを仮定している)
 *
 */
syscall_handler:
int64_handler:
		pushl	%ds
		pushl	%es
	
		pushl   %ebx
		pushl   %ecx
		pushl	%edx
		pushl	%esi
		pushl	%edi
/*		pushal */
		pushl 	%ebp
		movl	%esp,%ebp
		pushl	%edx	/* 引数の先頭番地 */
		pushl	%eax	/* システムコール番号 */
	
		movw	$0x10, %ax
		movw	%ax,%ds 
		movw	%ax,%es

		call 	syscall
	
		leave
/*		popal		 */
		popl	%edi
		popl	%esi
		popl	%edx
		popl    %ecx
		popl    %ebx

		popl	%es
		popl	%ds
#if 0
		iret
#else
		lret
#endif

/**************************************************************************
 * int65_handler --- POSIX system call.
 *
 *	システムコール割り込みの処理。
 *	そのため、他の割り込みと異なり、EOI などの処理を行わない。
 *
 *
 *	システムコール処理では次のことを「仮定」している。
 *	1) システムコール番号は、レジスタ eax に収められている。
 *	2) 同じくシステムコールへの引数の先頭番地は、レジスタ edx に
 *	   収められている。
 *	3) システムコールからの返り値は、レジスタ eax に収められる。
 *	   (ユーザプロセスはこのことを「知って」いることを仮定している)
 *
 */
posix_handler:
int65_handler:
		pushl	%ds
		pushl	%es
	
		pushl   %ebx
		pushl   %ecx
		pushl	%edx
		pushl	%esi
		pushl	%edi
/*		pushal */
		pushl 	%ebp
		movl	%esp,%ebp
	
		pushl	%edx	/* 引数の先頭番地 */
		pushl	%eax	/* システムコール番号 */
		pushl	$65	/* trap 番号 */
	
		movw	$0x10, %ax
		movw	%ax,%ds 
		movw	%ax,%es
	
		call 	trap

		leave		/* スタックに詰まれた引数は廃棄される */
/*		popal		 */
		popl	%edi
		popl	%esi
		popl	%edx
		popl    %ecx
		popl    %ebx
	
		popl	%es
		popl	%ds
#if 0
		iret
#else
		lret
#endif

#ifdef notdef	
		pushl 	%ebp
		movl	%esp,%ebp
		pushl	%edx	/* 引数の先頭番地 */
		pushl	%eax	/* システムコール番号 */
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
		movw	%ax,%fs
		movw	%ax,%gs		# and ds to gs
		call 	trap
		movw	$0x23, %ax
		movw	%ax,%ds
		movw	%ax,%es
		movw	%ax,%fs
		movw	%ax,%gs		# and ds to gs
		iret
#endif	

/**************************************************************************
 * int66_handler --- BTRON system call.
 *
 *	システムコール割り込みの処理。
 *	そのため、他の割り込みと異なり、EOI などの処理を行わない。
 *
 *	システムコール処理では次のことを「仮定」している。
 *	1) システムコール番号は、レジスタ eax に収められている。
 *	2) 同じくシステムコールへの引数の先頭番地は、レジスタ edx に
 *	   収められている。
 *	3) システムコールからの返り値は、レジスタ eax に収められる。
 *	   (ユーザプロセスはこのことを「知って」いることを仮定している)
 *
 */
_btron_handler:
int66_handler:
		pushl	%ds
		pushl	%es
	
		pushl   %ebx
		pushl   %ecx
		pushl	%edx
		pushl	%esi
		pushl	%edi
		pushl 	%ebp
		movl	%esp,%ebp
		pushl	%edx	/* 引数の先頭番地 */
		pushl	%eax	/* システムコール番号 */
		pushl	$66	/* trap 番号 */

		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es
#if 0
		movw	%ax,%fs
		movw	%ax,%gs		# and ds to gs
#endif

		call 	trap

		leave
		popl	%edi
		popl	%esi
		popl	%edx
		popl    %ecx
		popl    %ebx
	
		popl	%es
		popl	%ds
#if 0
		iret
#else
		lret
#endif
		

lock_count:	.word	0
/***************************************************************************
 *
 */
dis_int:
		cli
		ret

/*************************************************************************
 *
 */
ena_int:
		sti
		ret

/*****************************************************************************
 *
 */
open_interrupt:
		sti
		ret

	
	
.globl	call_syscall

/* call_syscall: 汎用型システムコールルーチン (obsolute)
 *
 * . スタック形式:
 *
 *		+--------------------+
 *		|    return addr     |	+4
 *		+--------------------+
 *		|    syscall no.     |  +8
 *		+--------------------+
 *		|    argument #1     |  +12
 *		+--------------------+
 *		|    argument #2     |  +16
 *		+--------------------+
 *		|    argument #3     |
 *
 */
call_syscall:
		pushl 	%ebp
		movl	%esp,%ebp
		movl	8(%ebp), %eax
		leal	12(%ebp), %edx
		int	$64
		leave
		ret
