/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* interrupt.h
 *
 *
 * Discription
 *
 */

#ifndef __IA32_INTERRUPT_H__
#define __IA32_INTERRUPT_H__	1

#define INTERRUPT_DESC	0x0e
#define TRAP_DESC	0x0f
#define FAULT_DESC	0x0f

#define INT_INVALID_OPCODE	6
#define INT_DOUBLE_FAULT	8
#define INT_TSS_FAULT		10
#define INT_INVALID_SEG		11
#define INT_STACK_SEG		12
#define INT_PROTECTION		13
#define INT_PAGE_FAULT		14

struct idt_t
{
  int	offset0:16;
  int	selector:16;
  int	zero:8;
  int	type:4;
  int	dt0:1;
  int	dpl:2;
  int	p:1;
  int	offset1:16;
};

#define GET_OFFSET_IDT(desc)		\
  (desc.offset0 | desc.offset1 << 16);

#define SET_OFFSET_IDT(desc,x)		\
(desc.offset0 = x & 0xffff);		\
  (desc.offset1 = x >> 16 & 0xffff);

extern W	on_interrupt;
extern BOOL	delayed_dispatch;

extern W	init_interrupt (void);
extern void	reset_intr_mask (W);
extern void	interrupt (W);
extern void	trap (W, W, VP);
extern void	page_fault (UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds,
		UW errcode, UW eip, UW cs, W eflags);
extern void	fault(UW intn, UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
	   UW ecx, UW eax, UW es, UW ds,
	   UW errcode, UW eip, UW cs, UW eflags);
extern W	wait_int (W *);
extern void	set_idt (UW, UW, UW, UW, UW);
extern void	protect_fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		   UW ecx, UW eax, UW es, UW ds,
		   UW errcode, UW eip, UW cs, UW eflags);
extern ER  	set_interrupt_entry (W intno, FP func, ATR attr);
extern W	int1_handler (void);
extern W	int2_handler (void);
extern W	int3_handler (void);
extern W	int4_handler (void);
extern W	int5_handler (void);
extern W	int6_handler (void);
extern W	int8_handler (void);
extern W	int10_handler (void);
extern W	int11_handler (void);
extern W	int12_handler (void);
extern W	int13_handler (void);
extern W	int14_handler (void);
extern W	int15_handler (void);
extern W	int16_handler (void);
extern W	int17_handler (void);
extern W	int18_handler (void);
extern W	int32_handler (void);
extern W	int33_handler (void);
extern W	int35_handler (void);
extern W	int37_handler (void);
extern W	int38_handler (void);	/* floppy disk */
extern W	int41_handler (void);	/* SCSI HD */
extern W	int42_handler (void);
extern W	int43_handler (void);
extern W        int44_handler (void);   /* psaux */
extern W	int46_handler (void);
extern W	syscall_handler(void);
extern W	posix_handler (void);
extern void	intr_interval (void);
extern void	start_interval (void);

#endif /* __IA32_INTERRUPT_H__ */


