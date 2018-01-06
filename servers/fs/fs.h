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
#include <stdbool.h>
#include <unistd.h>
#include <core.h>
#include <fs/vfs.h>
#include "vfs.h"
#include "devfs/devfs.h"
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

#endif				/* __FS_H__ */
