/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/

#ifndef __RD_H__
#define __RD_H__	1


#include <h/itron.h>
#include <h/types.h>
#include <h/errno.h>
#include <ibmpc/ibmpc.h>
#include <kernlib/device.h>

#define ROUNDDOWN(x,b)	((x / b) * b)
#define ROUNDUP(x,b)	((x + (b -1)) / b * b)

#define RD_DRIVER	"driver.ramdisk"

#define BLOCK_SIZE	512

#define RD_SIZE 	(2*1024*1024) /* 2 MByte */

#define DD_TYPE		1

/* main.c */
extern W    	open_rd();	/* オープン		*/
extern W  	close_rd();	/* クローズ		*/
extern W    	read_rd();	/* 読み込み		*/
extern W    	write_rd();	/* 書き込み		*/
extern W    	control_rd();	/* コントロール		*/

#endif /* __RD_H__ */
