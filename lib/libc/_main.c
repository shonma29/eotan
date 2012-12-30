/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Id: _main.c,v 1.6 2000/01/29 16:21:21 naniwa Exp $ */

/*
 * $Log: _main.c,v $
 * Revision 1.6  2000/01/29 16:21:21  naniwa
 * to clean up
 *
 * Revision 1.5  2000/01/18 14:39:03  naniwa
 * to make independent of libkernel.a
 *
 * Revision 1.4  2000/01/15 15:23:53  naniwa
 * to set file descriptor 0, 1 and 2
 *
 * Revision 1.3  1999/12/19 11:01:55  naniwa
 * modified to suit to libc
 *
 * Revision 1.2  1999/11/10 10:58:29  naniwa
 * correction
 *
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

#include <unistd.h>
#include "others/stdlib.h"

extern W main(W argc, B * argv[], B * envp[]);

void init_device();

W _main(W argc, B * argv[], B * envp[])
{
    int result;
    init_device();

    result = main(argc, argv, envp);
    _exit(result);

    return result;
}

static void InitFileTable(void);

FILE __file_table__[NFILE];

void init_device(void)
{
    InitFileTable();
    __file_table__[STDIN].device = 0;
    __file_table__[STDIN].count = 0;
    __file_table__[STDIN].length = 0;
    __file_table__[STDIN].bufsize = BUFSIZE;
    __file_table__[STDOUT].device = 1;
    __file_table__[STDOUT].count = 0;
    __file_table__[STDOUT].length = 0;
    __file_table__[STDOUT].bufsize = BUFSIZE;
    __file_table__[STDERR].device = 1;
    __file_table__[STDERR].count = 0;
    __file_table__[STDERR].length = 0;
    __file_table__[STDERR].bufsize = BUFSIZE;
}

static void InitFileTable(void)
{
    int i;

    /* initialize __file_table__[] */
    for (i = 0; i < NFILE; i++) {
	__file_table__[i].device = -1;
	__file_table__[i].count = 0;
	__file_table__[i].length = 0;
	__file_table__[i].bufsize = BUFSIZE;
    }
}
