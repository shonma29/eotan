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

#include <itron/types.h>

#define MAX_DPATH 255

struct lowlib_data
{
  ID	my_pid;		/* POSIX プロセス ID */

  ID	main_task;	/* ユーザプログラムのコードを実行するタスク */
  ID	signal_task;	/* シグナルの受信処理を行うタスク */
  
  B	dpath[MAX_DPATH+1]; /* カレントディレクトリ名 */
  W	dpath_len;	/* ディレクトリ名の長さ */

};

#define LOWLIB_DATA		((struct lowlib_data *)(0x7fff0000))

#endif /* __LOWLIB_H__ */
