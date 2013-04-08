/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* posix.h - POSIX 環境マネージャ用のヘッダファイル
 *
 *
 *
 */

#ifndef __FS_H__
#define __FS_H__	1


#include <errno.h>
#include <unistd.h>
#include <itron/rendezvous.h>
#include <core.h>
#include "../../kernel/misc.h"
#include "../console/console.h"

/* =========================== マクロ定義 =========================== */

/* 各種制限値 */
#define MAX_BODY_SIZE	1024
#define MAX_LINE_LENGTH	512
#define MAXFS		1
#define MAX_MOUNT	5
#define MAX_INODE	200
#define MAX_OPEN	100
#define MAX_NAMELEN	255
#define MAX_SIGNAL	32
#define MAX_PROCESS	100

#define POSIX_TASK_LEVEL	USER_LEVEL

/* macros for waitpid */
#define WNOHANG		1	/* 子プロセスの終了を待たない */
#define WUNTRACED	2	/* ジョブ制御用:未実装 */


struct timeval {
  long tv_sec;
  long tv_usec;
} ;

struct timezone {
  int tz_minuteswest;
  int tz_dsttime;
} ;





#include "api.h"
#include "sfs/sfs_fs.h"
#include "vfs.h"
#include "process.h"
#include "mm.h"


#include "../../lib/libserv/libserv.h"




/* ===========================  関数定義  =========================== */

/* request.c */
extern W init_port(void);
extern W get_request(struct posix_request *req);
extern W put_response(RDVNO rdvno, W errno, W status, W ret1);
extern W error_response(RDVNO rdvno, W errno);


#define printk dbg_printf


extern UW get_system_time(UW *usec);

#endif				/* __FS_H__ */
