/****************************************************************
 * interrupt.h
 * このコードは,B-Free Project の boot/2nd/interruput.h をもとにしたものです。
 *                                               最終変更 2000/5/1
 */

							   
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__		1

#define MASTER_8259A_COM	0x20
#define MASTER_8259A_DATA	0x21

#define SLAVE_8259A_COM		0xa0
#define SLAVE_8259A_DATA	0xa1

#define INT_KEYBOARD		33
#define	INT_FD			38		/* 1M type */
#define	INT_IDE			46		/* IDE HD */
#define	INT_IDE2		47		/* IDE HD */

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

void reset_intr_mask(int intn);

#endif /* __INTERRUPT_H__ */

 
