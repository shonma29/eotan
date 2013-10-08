/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/


/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/manager/vmtree.c,v 1.16 2000/01/26 08:24:36 naniwa Exp $ */


/* vmtree.c - 仮想メモリツリーの管理
 *
 * $Log: vmtree.c,v $
 * Revision 1.16  2000/01/26 08:24:36  naniwa
 * to prevent memory leak
 *
 * Revision 1.15  2000/01/15 15:29:31  naniwa
 * minor fix
 *
 * Revision 1.14  2000/01/08 09:10:27  naniwa
 * fixed initialization of variables, etc
 *
 * Revision 1.13  1999/12/26 11:17:40  naniwa
 * add debug write
 *
 * Revision 1.12  1999/11/10 10:48:35  naniwa
 * to implement execve
 *
 * Revision 1.11  1999/07/26 08:56:26  naniwa
 * minor fix
 *
 * Revision 1.10  1999/07/23 14:39:04  naniwa
 * modified to implement exit
 *
 * Revision 1.9  1999/03/24 03:54:47  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.8  1999/03/02 15:30:58  night
 * 田畑@KMC (yusuke@kmc.kyoto-u.ac.jp) のアドバイスによる修正。
 * --------------------
 * それから別のバグですが、
 * POSIX/manager/vmtree.cの grow_vm() の最初で pageentを
 * 求めるときに
 * pageent = addr % (MAX_PAGE_ENTRY * PAGE_SIZE) ;
 * となっていますが、ここは
 * pageent = (addr / PAGE_SIZE ) % MAX_PAGE_ENTRY ;
 * とか書くべきだと思います。
 * --------------------
 *
 * Revision 1.7  1998/07/01 14:03:10  night
 * デバッグ用のデータ出力処理を #ifdef notdef 〜 #endif で囲んだ。
 *
 * Revision 1.6  1998/02/25 12:53:07  night
 * システムコール vmap_reg の引数追加(アクセス権限を指定する引数を追加)に
 * 伴った修正。
 * アクセス権としては、ACC_USER (ユーザが読み書きできる) を指定した。
 *
 * Revision 1.5  1998/02/24 14:21:42  night
 * プロセスの間の仮想ページのデータのコピーする処理を
 * 正しく動くように修正。
 *
 * Revision 1.4  1998/02/23 14:50:51  night
 * setup_vmtree 関数をきちんと動くようにした。
 *
 * Revision 1.3  1998/02/16 14:28:31  night
 * setup_vmtree() の追加。
 * この関数は、引数で指定した ITRON タスクから仮想メモリテーブル
 * vmtree を生成する。
 *
 * Revision 1.2  1997/10/24 13:56:16  night
 * ファイル中の文字コードを SJIS から EUC に変更。
 *
 * Revision 1.1  1997/10/23 14:32:33  night
 * exec システムコール関係の処理の更新
 *
 *
 */

#include <kcall.h>
#include <string.h>
#include <mpu/config.h>
#include "fs.h"
#include "../../kernel/mpu/mpu.h"

#define VM_DEFAULT_ACCESS (VM_READ | VM_WRITE | VM_USER | VM_SHARE)
#define VM_DEFAULT_START (LOW_USER_ADDR)
#define VM_DEFAULT_SIZE (USER_MEM_SIZE)



/* grow_vm - 新しく仮想ページに物理メモリを割り当てる
 *
 */
W grow_vm(struct proc * procp, UW addr, UW access)
{
    ER error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    /* 仮想メモリ領域に物理メモリを割り付ける
     */
    error_no = kcall->region_map(procp->proc_maintask, (VP) addr, PAGE_SIZE, ACC_USER);
    if (error_no) {
	return (EPERM);
    }

    return (EOK);
}


/* shorten_vm - 仮想ページを解放する
 *
 */
W shorten_vm(struct proc * procp, UW addr)
{
    ER error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = kcall->region_unmap(procp->proc_maintask, (VP) addr, PAGE_SIZE);
    if (error_no) {
	return (EINVAL);
    }

    return (EOK);
}
