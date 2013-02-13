/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Id: device.c,v 1.4 1999/05/15 09:57:58 naniwa Exp $ */

/*
 * $Log: device.c,v $
 * Revision 1.4  1999/05/15 09:57:58  naniwa
 * modified to use wconsole
 *
 * Revision 1.3  1999/02/17 09:43:08  monaka
 * Modified for 'device descriptor'.
 *
 * Revision 1.2  1996/11/06 12:37:12  night
 * 実行開始時のコンソールクリア処理を変更した。
 *
 * Revision 1.1  1996/07/25  16:01:59  night
 * IBM PC 版用への最初の登録
 *
 * Revision 1.2  1995/12/05 14:32:41  night
 * init_device (void) の関数を追加。
 *
 * Revision 1.1  1995/10/10  16:21:47  night
 * 最初の登録.
 * システム起動後、プロンプトを出力してコマンド入力 - 実行ができるところ
 * まで出来た。ただし、コマンドは echo だけ。
 *
 *
 */

#include "init.h"

FILE __file_table__[NFILE];


void init_device(void)
{
    int i;

    /* initialize __file_table__[] */
    for (i = 0; i < NFILE; i++) {
	__file_table__[i].count = 0;
	__file_table__[i].length = 0;
	__file_table__[i].bufsize = BUFSIZE;
    }

    __file_table__[STDIN].device = 0;
    __file_table__[STDOUT].device = 1;
    __file_table__[STDERR].device = 1;
}
