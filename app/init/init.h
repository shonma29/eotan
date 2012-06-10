/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
(C) 2000-2003, Tomohide Naniwa

*/

#ifndef __INIT_H__
#define __INIT_H__	1

#include "../../include/itron/types.h"
#include "../../include/itron/errno.h"

#define MAJOR_VERSION	1
#define MINOR_VERSION	1


#define BUFSIZE		1024
#define MAXFILENAME	255
#define NFILE		10
#define MAXFSNAME	8
#define MAXDIRENTRY	100

#ifndef ROUNDUP
#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#endif


/*
 * SCSI デバイスの ID 2、パーティション 1 を root にする
 */
#define ROOT_DEV	"driver.scsi"
#define ROOT_PART	((0x2 << 7) | 1)
#define ROOT_TYPE	"dosfs"


#define STDIN	0
#define STDOUT	1
#define STDERR	2


struct file
{
  ID	device;
  W	count;
  W	length;
  W	bufsize;
  UB	buf[BUFSIZE];
};

typedef struct file	FILE;

extern FILE	__file_table__[];

#define stdin	(&__file_table__[0])
#define stdout	(&__file_table__[1])
#define stderr	(&__file_table__[2])



extern ID	dev_recv;	/* デバイスドライバからの返答用 */

extern void init_device(void);
extern ER open_device(B * dev_name, ID * id, W * dev_desc);

extern W	getc (FILE *port);
extern W	printf (B *fmt, ...);

#include "file.h"


#endif /* __INIT_H__ */
