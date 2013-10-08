/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* memory.c - POSIX 環境マネージャのメモリ処理部分
 *
 *
 *
 * $Log: memory.c,v $
 * Revision 1.4  1999/07/23 14:38:58  naniwa
 * modified to implement exit
 *
 * Revision 1.3  1997/10/24 13:57:42  night
 * duplicate_memory() と destroy_proc_memory () を追加。
 *
 * Revision 1.2  1997/10/23 14:32:32  night
 * exec システムコール関係の処理の更新
 *
 * Revision 1.1  1996/11/05 15:13:46  night
 * 最初の登録
 *
 */

#include <mpu/config.h>
#include "fs.h"


/* alloc_memory - プロセスに対して任意の仮想メモリを割り当てる。
 *
 * 引数：
 * 	procp	メモリを割り当てるプロセス(プロセス管理構造体へのポインタ)
 *	start	割り当てるメモリのアドレス (4KB alignment)
 *	size	メモリサイズ (4KB alignment)
 *	access	メモリのアクセスモード
 *
 */
W alloc_memory(W procid, UW start, UW size, UW access)
{
    W i;
    W error_no;
    struct proc *procp;


    error_no = proc_get_procp(procid, &procp);
    for (i = 0; i < (size >> PAGE_SHIFT); i++) {
	error_no = grow_vm(procp, start + (i << PAGE_SHIFT), access);
	if (error_no) {
	    return (error_no);
	}
    }

    return (EOK);
}
