/* startup.s
 *
 *	startup system.
 *
 */

#define GDT_ADDR	0x80001000
#define IDT_ADDR	0x80002000

.globl		startup

startup:
		cli
		/* idtr をセット */
		lidt	idt_ptr
		/* gdtr をセット */
		lgdt	gdt_ptr

		movl	$0x10, %eax
		mov	%ax, %ds
		mov	%ax, %es
		mov	%ax, %fs
		mov	%ax, %gs
		lss	stack_ptr, %esp

		finit	/* initialize FPU */

		pushl	$0
		pushl	$0
		pushl	$0
		cld

		call	itron			/* Go C world. */

.align 4
stack_ptr:
#if 0
		.long	0x00070000	/* offset */
#else
		.long	0x0009FFF0	/* offset */
#endif
		.word	0x10		/*8 selector */
			
.align 4
idt_ptr:
		.word	256*8-1		/* 256 entries * 8 --- size.	*/
		.long	IDT_ADDR	/* offset			*/

/*
	Global Descpritor Table.

	Entry:
	0x0000			0	Unused
	0x0008			1	kernel code (DPL = 0)
	0x0010			2	kernel data (DPL = 0)
	0x0018			3	application code (DPL = 3)
	0x0020			4	application data (DPL = 3)
	0x0028			5	application stack (DPL = 3)
	0x0030			6	ITRON call gate (DPL = 3)
	0x0038			7	POSIX call gate (DPL = 3)
	0x0040			8	BTRON call gate (DPL = 3)
	0x0048...0x03F8   9...127	unused
	0x0400		      128	unused
	0x0408		129...256	TSS (DPL = 0)
*/

.align 4
gdt_ptr:
		.word	256*8-1		/* 256 entries * 8 --- size. */
		.long	GDT_ADDR	/* offset. */

		
