/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* fs.h - ファイルシステム用のヘッダファイル
 *
 *
 *
 */

#ifndef __FS_H__
#define __FS_H__	1

#include <errno.h>
#include <unistd.h>
#include <core.h>
#include <fs/vfs.h>
#include "vfs.h"
#include "procfs/process.h"
#include "mm.h"
#include "sys/syscall.h"
#include "../../lib/libserv/libserv.h"

/* =========================== マクロ定義 =========================== */

#define EOK (0)

#define SU_UID		0

/* ===========================  関数定義  =========================== */

/* response.c */
extern W put_response(RDVNO rdvno, W error_no, W status, W ret1);
extern W put_response_long(RDVNO rdvno, W error_no, D status);

/* device.c */
extern W read_device(ID device, B * buf, W start, W length, W * rlength);
extern W write_device(ID device, B * buf, W start, W length, W * rlength);

/* cache.c */
extern void init_cache(void);
extern void get_cache(W fd, W blockno, W *cn, B **ptr);
extern void invalidate_cache(W fd, W blockno, W *cn);
extern void put_cache(W cn, W dirty);
extern W sync_cache(W fd, W umflag);

#endif				/* __FS_H__ */
