/****************************************************************
 * interrupt.h
 * このコードは,B-Free Project の boot/2nd/interruput.h をもとにしたものです。
 *                                               最終変更 2000/5/1
 */

							   
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__		1

#include "../../kernel/arch/arch.h"


extern void     int32_handler (void);
extern void	int33_handler (void);   /* keyboard */ 
extern void     int35_handler (void);
extern void	int36_handler (void);
extern void     int37_handler (void);
extern void	int38_handler (void);	/* floppy disk */
extern void	int39_handler (void);
extern void	int40_handler (void);
extern void	int41_handler (void);
extern void	int42_handler (void);
extern void	int43_handler (void);
extern void	int44_handler (void);
extern void	int45_handler (void);
extern void	int46_handler (void);   /* IDE 1 */
extern void	int47_handler (void);   /* IDE 2 */

extern void	init_8259A (void);
extern void	interrupt (int intn);
extern void	trap (int intn);
extern void	fault (int intn);
extern void	intr_keyboard (void);
extern void	open_interrupt (void);
extern void	ignore_handler (void);
extern void     ignore_interrupt_master (void);
extern void     ignore_interrupt_slave (void);
extern void     intr_fd();

extern int      wait_int (volatile int *flag);

extern void reset_intr_mask(int intn);

#endif /* __INTERRUPT_H__ */

 
