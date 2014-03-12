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
#include "api.h"
#include "sfs/sfs.h"
#include "vfs.h"
#include "process.h"
#include "mm.h"
#include "../../lib/libserv/libserv.h"

/* =========================== マクロ定義 =========================== */

#define EOK (0)

#define F_FILE		0x0001
#define F_PIPE		0x0002

#define printk dbg_printf

/* ===========================  関数定義  =========================== */

/* request.c */
extern W init_port(void);
extern W get_request(struct posix_request *req, RDVNO *rdvno);
extern W put_response(RDVNO rdvno, W error_no, W status, W ret1);
extern W error_response(RDVNO rdvno, W error_no);

extern UW get_system_time(void);

#endif				/* __FS_H__ */
