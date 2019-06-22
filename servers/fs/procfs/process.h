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
#include "../session.h"

/* exec.c */
extern W		exec_program (pm_args_t *, session_t *, vnode_t *, B *);

/* fork.c */
extern W		proc_duplicate (session_t *, session_t *);

/* init.c */
extern W exec_init(ID, char *);

#endif /* __FS_PROCESS_H__ */
