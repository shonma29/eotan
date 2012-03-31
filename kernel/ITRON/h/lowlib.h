/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/h/lowlib.h,v 1.5 2000/01/29 16:19:43 naniwa Exp $ */

#ifndef __LOWLIB_H__
#define __LOWLIB_H__	1


#include "itron_module.h"

/* LOWLIB 関連のシステムコール
 */
#define LOD_LOW		(3)
#define ULD_LOW		(4)
#define STS_LOW		(5)


struct lowlib_info
{
  struct module_info	*modp;
  B			name[MAX_MODULE_NAME];
  W			intr;

  ER			(*start)(VP stack_top);
  ER			(*quit)(void);
  ER			(*intr_func)(W signo, VP arg);	/* 割り込み処理関数 */
};

#define MAX_DPATH 255

struct lowlib_data
{
  ID	recv_port;	/* POSIX マネージャへの要求の返答受け付け用ポート */

  ID	my_pid;		/* POSIX プロセス ID */

  ID	main_task;	/* ユーザプログラムのコードを実行するタスク */
  ID	signal_task;	/* シグナルの受信処理を行うタスク */
  ID	fifo_task;	/* パイプを使うときに使用するタスク */
  ID	alarm_task;	/* alarm システムコール用のタスク(POSIX 環境用) */
  
  FP	start_func;	/* プロセスのスタート番地 */

  ID	efile;		/* 実行ファイルを指しているメッセージポート 
			 * コード部のページインのときに使用する。
			 */

  W	errno;		/* エラー番号記録用 */

  B	dpath[MAX_DPATH+1]; /* カレントディレクトリ名 */
  W	dpath_len;	/* ディレクトリ名の長さ */

};

#define LOWLIB_DATA		((struct lowlib_data *)(0x7fff0000))


#ifdef __ITRON__

extern struct lowlib_info	lowlib_table[MAX_MODULE];

extern ER			init_lowlib (struct module_info *modp);
extern ER			load_lowlib (VP *argp);
extern ER			unload_lowlib (VP *argp);


#endif /* __ITRON__ */


#endif /* __LOWLIB_H__ */
