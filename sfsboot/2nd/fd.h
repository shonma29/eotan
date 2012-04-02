/**************************************************************************
* fd.h
* このコードは,B-Free Project の boot/2nd/fd.h をもとにしたものです。 
* 
*                                                最終変更 2000/5/1  
*/


#ifndef __FD_H__
#define __FD_H__	

#include "types.h"

extern int	init_fd (void);
extern void	intr_fd (void);
extern int	fd_ready_check (void);
extern int	on_motor (BYTE drive);
extern int	stop_motor (BYTE drive);
extern int	fd_get_status (BYTE drive, int datan);
extern int	fd_recalibrate (BYTE drive);
extern int	fd_seek (BYTE drive, int head, int cylinder, int motor);
extern int	fd_specify (UWORD32,UWORD32,UWORD32,UWORD32);
extern int	fd_reset (void);
extern int      fdc_isense(void);
extern int      fdc_read_sector(BYTE drive, int cylinder, int head, int sector, BYTE* buff);
extern int      fd_read (int drive, int part, int blockno, BYTE *buff, int length);

#endif /* __FD_H__ */

