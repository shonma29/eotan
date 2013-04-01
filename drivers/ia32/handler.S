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


.globl  int32_handler
.globl  int33_handler
.globl  int44_handler
.globl	int46_handler

.text

/**************************************************************************
 *
 * int32_handler --- timer 
 *
 *
 */
int32_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es

		inb	MASTER_8259A_DATA, %al
		orb	$1, %al
		outb	%al, MASTER_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		sti

		pushl	$32
		call	interrupt
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret

/**************************************************************************
 * int33_handler --- keyboard
 *
 */
int33_handler:
		pushl	%ds
		pushl	%es
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds
		movw	%ax,%es

		inb	MASTER_8259A_DATA, %al
		orb	$2, %al
		outb	%al, MASTER_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		sti

		pushl	$33
		call	interrupt
		popl	%eax
		popal
		popl	%es
		popl	%ds
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
/* original by Mr. Murata */
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

		inb	SLAVE_8259A_DATA, %al
		orb	$0x40, %al
		outb	%al, SLAVE_8259A_DATA
		movb	$0x20, %al
		outb	%al, MASTER_8259A_COM
		jmp	int46_out	# delay
int46_out:	outb	%al, SLAVE_8259A_COM
		sti

		pushl	$46
		call	interrupt
		popl	%eax
		popal
		popl	%es
		popl	%ds
		iret

