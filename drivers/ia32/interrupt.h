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

#endif /* __IA32_INTERRUPT_H__ */


