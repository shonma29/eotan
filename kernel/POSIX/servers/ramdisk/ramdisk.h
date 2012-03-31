/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/

#ifndef __RD_H__
#define __RD_H__	1


#include "h/itron.h"
#include <h/types.h>
#include <h/errno.h>
#include <ibmpc/ibmpc.h>
#include "device.h"

#define ROUNDDOWN(x,b)	((x / b) * b)
#define ROUNDUP(x,b)	((x + (b -1)) / b * b)

#define RD_DRIVER	"driver.ramdisk"

#define BLOCK_SIZE	512

#define RD_SIZE 	(2*1024*1024) /* 2 MByte */

#define DD_TYPE		1

/* main.c */
extern W    	open_rd();	/* �����ץ�		*/
extern W  	close_rd();	/* ������		*/
extern W    	read_rd();	/* �ɤ߹���		*/
extern W    	write_rd();	/* �񤭹���		*/
extern W    	control_rd();	/* ����ȥ���		*/

#endif __RD_H__
