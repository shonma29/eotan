/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* posix_syscall.h - POSIX 環境マネージャ用のヘッダファイル
 *		     (システムコール関連の定義)
 *
 * Note:
 *	PSC (and psc) = Posix System Call
 *
 */

#ifndef __SYS_SYSCALL_H__
#define __SYS_SYSCALL_H__	1

#include <core/types.h>
#include <sys/packets.h>
#include <fs/fscall.h>

/* =========================== 構造体定義 =========================== */


/* POSIX マネージャへの要求メッセージの構造体 */
struct posix_request
{
  ID	procid;			/* 呼び出し元のプロセス ID */
  enum FsCall operation;		/* 要求番号(システムコールに対応)	*/
  struct psc_args args;
};



struct posix_response
{
  W	error_no;			/* エラー番号 */
  W	status;			/* ステータス(リターン値) */
  W	ret1;			/* サブに使用するリターン値 */
};

#endif /* #define __SYS_SYSCALL_H__ */
