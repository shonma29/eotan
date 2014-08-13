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
#define PSC_STATVFS       20
#define PSC_UMOUNT       21

/* =================== SIGNAL 関係 =============== */
#define PSC_KILL         22

/* =================== miserous system calls  =============== */
#define PSC_DUP2         23	/* 不要? */

#define PSC_BIND_DEVICE (24)

/* =============== 各システムコール用の 構造体定義 ================== */

/* psc_chdir -
 *
 */
struct psc_chdir
{
  W		pathlen;
  B		*path;
};


/* psc_chmod -
 *
 */
struct psc_chmod
{
  W		pathlen;
  B		*path;
  W		mode;
};


/* psc_close -
 *
 */
struct psc_close
{
  W		fileid;
};


/* psc_dup -
 *
 */
struct psc_dup
{
  W		fileid;
};


/* psc_dup2 -
 *
 */
struct psc_dup2
{
  W		fileid1;
  W		fileid2;
};


/* psc_execve - 指定したプログラムファイルを読み込み、実行する
 *
 */
struct psc_execve
{
  UW		pathlen;
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



/* psc_kill -
 *
 */
struct psc_kill
{
  W		pid;
  W		signal;
};


/* psc_link -
 *
 */
struct psc_link
{
      W srclen;
      B *src;
      W dstlen;
      B *dst;
};


/* psc_lseek -
 *
 */
struct psc_lseek
{
  W		fileid;
  W		offset;
  W		mode;
};


/* psc_mkdir -
 *
 */
struct psc_mkdir
{
  W		pathlen;
  B		*path;
  W		mode;
};


/* psc_open -
 *
 */
struct psc_open
{
  W		pathlen;
  B		*path;
  W		oflag;
  W		mode;	/* if oflag is O_CREATE */
};


/* psc_read -
 *
 */
struct psc_read
{
  W	fileid;
  B	*buf;
  W	length;
};


/* psc_rmdir -
 *
 */
struct psc_rmdir
{
  W		pathlen;
  B		*path;
};


/* psc_fstat -
 *
 */
struct psc_fstat
{
  W		fileid;
  struct stat	*st;
};


/* psc_unlink -
 *
 */
struct psc_unlink
{
  W		pathlen;
  B		*path;
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


/* psc_write -
 *
 */
struct psc_write
{
  W	fileid;
  B	*buf;
  W	length;
};


/* psc_mount -
 *
 */
struct psc_mount
{
  W	devnamelen;    		/* マウントするデバイス名の長さ */
  B	*devname;		/* マウントするデバイス名の長さ */
  W	dirnamelen;		/* マウントするディレクトリ名の長さ */
  B	*dirname;		/* マウントするディレクトリ名 */
  W	option;			/* オプション */
  W	fstypelen;
  B	*fstype;		/* ファイルシステムのタイプ */
};


/* psc_umount -
 *
 */
struct psc_umount
{
  W	dirnamelen;
  B	*dirname;
};


/* psc_statvfs -
 *
 */
struct psc_statvfs
{
  ID		device;
  struct statvfs	*fsp;
};


/* psc_getdents -
 *
 */
struct psc_getdents
{
  UW fileid;
  VP buf;
  UW length;
};

#define MAX_DEVICE_NAME (31)
struct psc_bind_device
{
  UW id;
  UB name[MAX_DEVICE_NAME + 1];
  ID port;
  UW size;
};

/* =========================== 構造体定義 =========================== */


/* POSIX マネージャへの要求メッセージの構造体 */
struct posix_request
{
  ID	procid;			/* 呼び出し元のプロセス ID */
  UW	operation;		/* 要求番号(システムコールに対応)	*/

  union {
    struct psc_chdir		par_chdir;
    struct psc_chmod		par_chmod;
    struct psc_close		par_close;
    struct psc_dup		par_dup;
    struct psc_dup2		par_dup2;
    struct psc_execve		par_execve;
    struct psc_exit		par_exit;
    struct psc_fcntl		par_fcntl;
    struct psc_fork		par_fork;
    struct psc_kill		par_kill;
    struct psc_link		par_link;
    struct psc_lseek		par_lseek;
    struct psc_mkdir		par_mkdir;
    struct psc_open		par_open;
    struct psc_read		par_read;
    struct psc_rmdir		par_rmdir;
    struct psc_fstat  		par_fstat;
    struct psc_unlink     	par_unlink;
    struct psc_waitpid   	par_waitpid;
    struct psc_write 		par_write;
    struct psc_mount		par_mount;
    struct psc_umount		par_umount;
    struct psc_statvfs		par_statvfs;
    struct psc_getdents		par_getdents;
    struct psc_bind_device par_bind_device;
  } param;
};



struct posix_response
{
  W	error_no;			/* エラー番号 */
  W	status;			/* ステータス(リターン値) */
  W	ret1;			/* サブに使用するリターン値 */
};

#endif /* #define __SYS_SYSCALL_H__ */
