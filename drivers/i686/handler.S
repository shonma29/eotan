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

.text

/**************************************************************************
 *
 * int32_handler --- timer 
 *
 *
 */
int32_handler:
		pushl	%ds
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds

		pushl	$32
		call	interrupt
		addl	$4, %esp

		movb	$0x60, %al
		outb	%al, MASTER_8259A_COM
		sti

		popal
		popl	%ds
		iret

/**************************************************************************
 * int33_handler --- keyboard
 *
 */
int33_handler:
		pushl	%ds
		pushal
		movw	$0x10, %ax
		movw	%ax,%ds

		pushl	$33
		call	interrupt
		addl	$4, %esp

		movb	$0x61, %al
		outb	%al, MASTER_8259A_COM
		sti

		popal
		popl	%ds
		iret
