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

/* =================== POSIX システムコール番号の定義 =============== */
#define PSC_CHDIR        0
#define PSC_CHMOD        1
#define PSC_CLOSE        2
#define PSC_DUP          3
#define PSC_EXEC         4
#define PSC_EXIT         5
#define PSC_FCNTL        6
#define PSC_FORK         7
#define PSC_FSTAT	 8
#define PSC_LINK         9
#define PSC_LSEEK        10
#define PSC_MKDIR        11
#define PSC_OPEN         12
#define PSC_READ         13
#define PSC_RMDIR        14
#define PSC_UNLINK       15
#define PSC_WAITPID      16
#define PSC_WRITE        17
#define PSC_GETDENTS	 18
#define PSC_MOUNT        19
#define PSC_STATVFS      20
#define PSC_UNMOUNT      21

/* =================== SIGNAL 関係 =============== */
#define PSC_KILL         22

/* =================== miserous system calls  =============== */
#define PSC_DUP2         23	/* 不要? */

/* =============== 各システムコール用の 構造体定義 ================== */

/* psc_chdir -
 *
 */
struct psc_chdir
{
  B		*path;
};


/* psc_execve - 指定したプログラムファイルを読み込み、実行する
 *
 */
struct psc_execve
{
  B		*name;
  B		*stackp;
  W		stsize;
};


/* psc_exit -
 *
 */
struct psc_exit
{
  W		evalue;
};


/* psc_fcntl -
 *
 */
struct psc_fcntl
{
  W fileid;
  W cmd;
  VP arg;
};


/* psc_fork -
 *
 */
struct psc_fork
{
  VP sp;
  FP entry;
};



/* psc_open -
 *
 */
struct psc_open
{
  B		*path;
  W		oflag;
  W		mode;	/* if oflag is O_CREATE */
};


/* psc_waitpid -
 *
 */
struct psc_waitpid
{
  W pid;
  W *statloc;
  W opts;
};


/* psc_statvfs -
 *
 */
struct psc_statvfs
{
  ID		device;
  struct statvfs	*fsp;
};


#define MAX_DEVICE_NAME (31)

/* =========================== 構造体定義 =========================== */


/* POSIX マネージャへの要求メッセージの構造体 */
struct posix_request
{
  ID	procid;			/* 呼び出し元のプロセス ID */
  UW	operation;		/* 要求番号(システムコールに対応)	*/
  struct psc_args args;
  union {
    struct psc_chdir		par_chdir;
    struct psc_execve		par_execve;
    struct psc_exit		par_exit;
    struct psc_fcntl		par_fcntl;
    struct psc_fork		par_fork;
    struct psc_open		par_open;
    struct psc_waitpid   	par_waitpid;
    struct psc_statvfs		par_statvfs;
  } param;
};



struct posix_response
{
  W	error_no;			/* エラー番号 */
  W	status;			/* ステータス(リターン値) */
  W	ret1;			/* サブに使用するリターン値 */
};

#endif /* #define __SYS_SYSCALL_H__ */
