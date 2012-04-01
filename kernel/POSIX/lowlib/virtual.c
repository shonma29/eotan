/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/virtual.c,v 1.1 1996/11/11 13:36:06 night Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/virtual.c,v 1.1 1996/11/11 13:36:06 night Exp $";

/*
 * 仮想記憶関連の処理を行うファイル。
 *
 *
 * $Log: virtual.c,v $
 * Revision 1.1  1996/11/11 13:36:06  night
 * IBM PC 版への最初の登録
 *
 * ----------------
 *
 *
 */

#include "lowlib.h"

/* vm_create - ユーザプロセスの仮想空間を作成する。
 *
 */
int
vm_create (ID taskid, UW vaddr, UW size, UW permission, UW mode)
{
}

