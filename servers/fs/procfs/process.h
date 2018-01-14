/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/

/* posix_proc.h - POSIX プロセス管理
 *
 *
 * $Log: posix_proc.h,v $
 * Revision 1.16  2000/05/06 03:52:26  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.15  2000/01/15 15:29:29  naniwa
 * minor fix
 *
 * Revision 1.14  1999/07/23 14:39:01  naniwa
 * modified to implement exit
 *
 * Revision 1.13  1999/07/21 15:10:06  naniwa
 * modified to implement waitpid
 *
 * Revision 1.12  1999/03/24 04:52:09  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.11  1998/05/23 15:32:43  night
 * enum 宣言の最後の要素についていた ',' を削除した。
 * (ANSI の文法では、最後の要素に は、',' はつけられない)
 *
 * Revision 1.10  1998/01/06 16:37:11  night
 * proc 構造体にプロセスが使っているメモリ領域の情報用のエントリを追加し
 * た。
 *
 * テキスト領域
 * UW			text_start_addr;
 * UW			text_length;
 *
 * データ領域
 * UW			data_start_addr;
 * UW			data_length;
 *
 * BSS 領域
 * UW			bss_start_addr;
 * UW			bss_length;
 *
 * Revision 1.9  1997/12/17 14:16:21  night
 * 関数 fork() の引数変更に伴う、関数プロトタイプの変更。
 *
 * Revision 1.8  1997/10/24 13:59:26  night
 * enum proc_status の追加。
 * 関数定義の追加。
 *
 * Revision 1.7  1997/10/23 14:32:33  night
 * exec システムコール関係の処理の更新
 *
 * Revision 1.6  1997/05/08 15:11:30  night
 * プロセスの情報を設定する機能の追加。
 * (syscall misc の proc_set_info コマンド)
 *
 * Revision 1.5  1997/03/25 13:34:53  night
 * ELF 形式の実行ファイルへの対応
 *
 * Revision 1.4  1996/11/18  13:43:47  night
 * ファイル構造体にファイルの種類(通常ファイルかパイプか)を示す、
 * f_flag を追加。
 *
 * Revision 1.3  1996/11/17  16:55:28  night
 * 文字コードを EUC にした。
 *
 * Revision 1.2  1996/11/17  16:47:49  night
 * 以下の要素を追加
 *
 *    struct proc		*proc_next;
 *    W			proc_euid;
 *    W			proc_egid;
 *    W			proc_umask;
 *
 * Revision 1.1  1996/11/14  13:18:15  night
 * 最初の登録
 *
 *
 */

#ifndef __FS_PROCESS_H__
#define __FS_PROCESS_H__	1

#include <fs/config.h>
#include <sys/syscall.h>
#include "../session.h"

enum proc_status
{
  PS_DORMANT = 0,		/* 未生成状態 */
  PS_SLEEP = 1,		/* sleep 状態 */
  PS_WAIT = 2,		/* wait 状態 */
  PS_RUN = 3,		/* run 状態 */
  PS_ZOMBIE = 4,		/* zombie 状態 */
  PS_TRANSITION = 5
};

struct proc
{
  struct proc		*proc_next;

  enum proc_status	proc_status;		/* プロセスの状態を示す */

  ID			proc_maintask;		/* メインタスク */

  session_t session;

  UW			proc_pid;		/* my process ID 
						 * この値が 0 のときは、このエントリは、
						 * 使っていない。
						 */
  
  UW			proc_ppid;		/* parent process ID */

  UW			proc_pgid;		/* process group ID */
  UW			proc_wpid;		/* pid parameter of waitpid */
  RDVNO proc_wait_rdvno;
  UW			proc_exst;		/* exit status */

  char			proc_name[PROC_NAME_LEN];
};


/* process.c */

extern struct proc	proc_table[MAX_PROCESS];

extern W		init_process (void);
extern W		proc_get_permission (W procid, struct permission *p);
extern W		proc_get_pid (W procid, W *pid);
extern W		proc_get_ppid (W procid, W *ppid);
extern W proc_get_status(W procid);
extern W		proc_alloc_fileid (W procid, W *retval);
extern W		proc_get_cwd (W procid, vnode_t **cwd);
extern W		proc_set_cwd (W procid, vnode_t *cwd);
extern W		proc_get_file (W procid, W fileid, struct file **fp);
extern W		proc_set_file (W procid, W fileid, W flag, vnode_t *ip);
extern W		proc_get_procp (W procid, struct proc **procp);
extern W		proc_exit (W procid);
extern W		proc_alloc_proc (struct proc **procp);
extern void proc_dealloc_proc(W procid);


/* exec.c */
extern W		exec_program (struct posix_request *req, W procid, B *pathname);

/* fork.c */
extern W		proc_fork (struct proc *parent, struct proc *child);
extern W copy_local(struct proc * parent, struct proc * child);

/* vfs.c */
extern W		open_special_devices(struct proc *procp);

/* init.c */
extern W exec_init(ID process_id, char *pathname);

#endif /* __FS_PROCESS_H__ */
