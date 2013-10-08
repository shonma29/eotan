/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* posix_mm.h - POSIX メモリサブシステム
 *
 *
 * $Log: posix_mm.h,v $
 * Revision 1.6  2000/01/08 09:09:16  naniwa
 * add some prototypes
 *
 * Revision 1.5  1999/07/23 14:39:00  naniwa
 * modified to implement exit
 *
 * Revision 1.4  1998/02/16 14:22:49  night
 * vm_tree 構造体の要素 vm_handler の型を変更。
 * W (*)() から FP に変更した。
 *
 * Revision 1.3  1997/10/24 13:59:11  night
 * VM_EXEC、VM_EXEC_MASK マクロの追加。
 * 関数定義の追加。
 *
 * Revision 1.2  1997/10/23 14:32:32  night
 * exec システムコール関係の処理の更新
 *
 * Revision 1.1  1996/11/13 12:53:35  night
 * 最初の登録
 *
 *
 */

#ifndef __FS_MM_H__
#define __FS_MM_H__	1

#include <vm.h>

/* memory.c */
extern W			alloc_memory (W procid, UW start, UW size, UW access);

/* vmtree.c */
extern W			grow_vm (struct proc *procp, UW addr, UW access);
extern W			shorten_vm (struct proc *procp, UW addr);

/* init.c */
extern W exec_init(ID process_id, char *pathname);

#endif /* __FS_MM_H__ */
