/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
 */

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/exit.c,v 1.1 1996/11/11 13:33:00 night Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/exit.c,v 1.1 1996/11/11 13:33:00 night Exp $";


/* $Log: exit.c,v $
/* Revision 1.1  1996/11/11 13:33:00  night
/* 最初の登録
/*
 * Revision 1.2  1995/09/21  15:52:10  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.1  1995/08/21  13:20:47  night
 * 最初の登録
 *
 *
 */

#include <sys/types.h>
#include <native/syscall.h>


/*
 * プログラムの処理を終了する。
 */
exit (int exitcode)
{
  shutdown_stdio ();	/* 標準入出力ライブラリの後始末 */
                        /* バッファをフラッシュする */

  _exit (exitcode);
}

