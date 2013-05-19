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

#include <itron/types.h>

#define NR_POSIX_SYSCALL	43

/* =================== POSIX システムコール番号の定義 =============== */
#define PSC_NOACTION     0
#define PSC_ACCESS       1
#define PSC_CHDIR        2
#define PSC_CHMOD        3
#define PSC_CHOWN        4
#define PSC_CLOSE        5
#define PSC_DUP          6
#define PSC_EXEC         7
#define PSC_EXIT         8
#define PSC_FCNTL        9
#define PSC_FORK         10
#define PSC_FSTAT	 11
#define PSC_GETEGID      12
#define PSC_GETEUID      13
#define PSC_GETGID       14
#define PSC_GETPID       15
#define PSC_GETPPID      16
#define PSC_GETUID       17
#define PSC_LINK         18
#define PSC_LSEEK        19
#define PSC_MKDIR        20
#define PSC_OPEN         21
#define PSC_READ         22
#define PSC_RMDIR        23
#define PSC_SETGID       24
#define PSC_SETUID       25
#define PSC_TIME         26
#define PSC_UMASK        27
#define PSC_UNLINK       28
#define PSC_UTIME        29
#define PSC_WAITPID      30
#define PSC_WRITE        31

#define PSC_BRK		 32
#define PSC_GETDENTS	 33
#define PSC_MOUNT        34
#define PSC_STATFS       35
#define PSC_UMOUNT       36

/* =================== SIGNAL 関係 =============== */
#define PSC_KILL         37

/* =================== miserous system calls  =============== */
#define PSC_MISC         38

#define PSC_DUP2         39	/* 不要? */

#define PSC_BIND_DEVICE (40)

/* MISC 
 */
#define M_PROC_DUMP	2
#define M_VM_DUMP	3
#define M_PRINT_FLIST	4
#define M_PURGE_CACHE	5
#define M_DO_PS		7
#define M_DO_DF		8

/* =============== 各システムコール用の 構造体定義 ================== */

/* psc_access -
 *
 */
struct psc_access
{
  W		pathlen;
  B		*path;
  W		accflag;
};


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


/* psc_chown -
 *
 */
struct psc_chown
{
  W		pathlen;
  B		*path;
  W		uid;
  W		gid;
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
#if 0
  B		(*argv)[];
  B		(*envp)[];
#else
  B		*stackp;
  W		stsize;
#endif

#if 0
  FP		start_main;	/* メインタスク用のスタート関数 */
  FP		start_signal;	/* シグナルタスク用のスタート関数 */
#endif
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



/* psc_getegid -
 *
 */
struct psc_getegid
{
};


/* psc_geteuid -
 *
 */
struct psc_geteuid
{
  /* have no value */
};


/* psc_getgid -
 *
 */
struct psc_getgid
{
  /* have no value */
};


/* psc_getpid -
 *
 */
struct psc_getpid
{
};


/* psc_getppid -
 *
 */
struct psc_getppid
{
};


/* psc_getuid -
 *
 */
struct psc_getuid
{
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


/* psc_setgid -
 *
 */
struct psc_setgid
{
  UW		gid;
};


/* psc_setuid -
 *
 */
struct psc_setuid
{
  UW		uid;
};


/* psc_fstat -
 *
 */
struct psc_fstat
{
  W		fileid;
  struct stat	*st;
};


/* psc_time - 実態は gettimeofday
 *
 */
struct psc_time
{
  VP tv;
  VP tz;
};


/* psc_umask -
 *
 */
struct psc_umask
{
  W	umask;
};


/* psc_unlink -
 *
 */
struct psc_unlink
{
  W		pathlen;
  B		*path;
};


/* psc_utime -
 *
 */
struct psc_utime
{
      W pathlen;
      B *path;
      struct utimbuf *buf;
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
#ifdef notdef
  W	option;
#endif
};


/* psc_statfs -
 *
 */
struct psc_statfs
{
  ID		device;
  struct statfs	*fsp;
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

/* psc_brk -
 *
 */

struct psc_brk
{
  VP end_adr;
};

/* psc_misc
 *
 */
struct psc_misc
{
  W		cmd;
  W		length;
  union
    {
      W			procid;
    } arg;
};

#define MAX_DEVICE_NAME (31)
struct psc_bind_device
{
  UW id;
  UB name[MAX_DEVICE_NAME + 1];
  ID port;
};

/* =========================== 構造体定義 =========================== */


/* POSIX マネージャへの要求メッセージの構造体 */
struct posix_request
{
  ID	caller;			/* 呼び出し元のタスク ID */
  ID	procid;			/* 呼び出し元のプロセス ID */
  W	msg_length;		/* メッセージの長さ	*/
  W	operation;		/* 要求番号(システムコールに対応)	*/

  union {
    struct psc_access		par_access;
    struct psc_brk		par_brk;
    struct psc_chdir		par_chdir;
    struct psc_chmod		par_chmod;
    struct psc_chown		par_chown;
    struct psc_close		par_close;
    struct psc_dup		par_dup;
    struct psc_dup2		par_dup2;
    struct psc_execve		par_execve;
    struct psc_exit		par_exit;
    struct psc_fcntl		par_fcntl;
    struct psc_fork		par_fork;
    struct psc_getegid		par_getegid;
    struct psc_geteuid		par_geteuid;
    struct psc_getgid		par_getgid;
    struct psc_getpid		par_getpid;
    struct psc_getppid		par_getppid;
    struct psc_getuid		par_getuid;
    struct psc_kill		par_kill;
    struct psc_link		par_link;
    struct psc_lseek		par_lseek;
    struct psc_mkdir		par_mkdir;
    struct psc_open		par_open;
    struct psc_read		par_read;
    struct psc_rmdir		par_rmdir;
    struct psc_setgid		par_setgid;
    struct psc_setuid		par_setuid;
    struct psc_fstat  		par_fstat;
    struct psc_time  		par_time;
    struct psc_umask 		par_umask;
    struct psc_unlink     	par_unlink;
    struct psc_utime 		par_utime;
    struct psc_waitpid   	par_waitpid;
    struct psc_write 		par_write;
    struct psc_mount		par_mount;
    struct psc_umount		par_umount;
    struct psc_statfs		par_statfs;
    struct psc_misc		par_misc;
    struct psc_getdents		par_getdents;
    struct psc_bind_device par_bind_device;
  } param;
};



struct posix_response
{
  W	msg_length;		/* メッセージの長さ	*/
  W	errno;			/* エラー番号 */
  W	status;			/* ステータス(リターン値) */

  W	ret1;			/* サブに使用するリターン値 */
};

#endif /* #define __SYS_SYSCALL_H__ */
