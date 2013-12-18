/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/


/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/manager/fork.c,v 1.15 2000/05/06 03:52:24 naniwa Exp $ */


/* fork.c --- プロセスの fork を行う
 *
 *
 * $Log: fork.c,v $
 * Revision 1.15  2000/05/06 03:52:24  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.14  2000/02/27 15:33:39  naniwa
 * minor change
 *
 * Revision 1.13  2000/01/26 08:24:33  naniwa
 * to prevent memory leak
 *
 * Revision 1.12  2000/01/21 14:22:30  naniwa
 * fixed copying file descriptors, etc
 *
 * Revision 1.11  2000/01/15 15:29:28  naniwa
 * minor fix
 *
 * Revision 1.10  2000/01/08 09:08:40  naniwa
 * fixed to copy working directory
 *
 * Revision 1.9  1999/11/10 10:48:09  naniwa
 * minor fix
 *
 * Revision 1.8  1999/07/23 14:39:48  naniwa
 * modfied to call lod_low
 *
 * Revision 1.7  1999/07/21 15:10:04  naniwa
 * modified to implement waitpid
 *
 * Revision 1.6  1999/03/24 03:54:44  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.5  1998/02/24 14:12:48  night
 * デバッグ用プリント文の追加。
 *
 * Revision 1.4  1998/02/23 14:48:14  night
 * デバッグ用プリント文の追加。
 *
 * Revision 1.3  1998/02/16 14:21:31  night
 * デバッグ用プリント文の追加。
 *
 * Revision 1.2  1997/12/17 14:15:47  night
 * 関数 fork() の中身を作った。
 * でもまだ、完成していない。
 *
 * Revision 1.1  1997/10/24 14:01:40  night
 * 最初の登録
 *
 *
 *
 */

#include <local.h>
#include <string.h>
#include <nerve/kcall.h>
#include "fs.h"
#include "../../lib/libserv/libmm.h"

static W proc_duplicate(struct proc * source, struct proc * destination);


/* fork - プロセスの fork を行う
 *
 *
 */
W proc_fork(struct proc *parent, struct proc *child)
{
    W error_no;

#ifdef FKDEBUG
    printk
	("fork: call alloc_proc: (%s file, %d line), parent = 0x%x, parent->vm_tree = 0x%x\n",
	 __FILE__, __LINE__, parent, parent->vm_tree);	/* */
#endif

    /* プロセス情報のコピー */
    /* 管理情報 (プロセスおよびファイル)の更新 */
#ifdef FKDEBUG
    printk("fork(): parent = 0x%x, parent->vm_tree = 0x%x\n", parent, parent->vm_tree);	/* */
    printk("fork(): child = 0x%x, child->vm_tree = 0x%x\n", child, child->vm_tree);	/* */
#endif
    error_no = proc_duplicate(parent, child);
    if (error_no) {
	return (error_no);
    }

    child->proc_status = PS_RUN;
    child->proc_ppid = parent->proc_pid;
    strncpy(child->proc_name, parent->proc_name, PROC_NAME_LEN - 1);
    child->proc_name[PROC_NAME_LEN - 1] = '\0';

    return (EOK);
}

W copy_local(struct proc *parent, struct proc *child)
{
    thread_local_t local_data;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;
    W error_no = vmap(child->proc_pid, (thread_local_t*)LOCAL_ADDR,
		sizeof(thread_local_t), true);

    if (error_no) {
	return error_no;
    }

    error_no = kcall->region_get(parent->proc_maintask, (thread_local_t*)LOCAL_ADDR,
		     sizeof(thread_local_t), &local_data);
    if (error_no) {
//TODO release local
	return error_no;
    }

    local_data.thread_id = child->proc_maintask;
    local_data.process_id = child->proc_pid;

    error_no = kcall->region_put(child->proc_maintask, (thread_local_t*)LOCAL_ADDR,
		     sizeof(thread_local_t), &local_data);
    if (error_no) {
//TODO release local
	return error_no;
    }

    return (EOK);
}

/* proc_duplicate - プロセス情報のコピー
 *
 *	Lowlib についての情報は、呼び出しもとで設定しているので必要ない。
 *	(タスクの実行開始アドレスは、Lowlib 中の fork 用関数に設定されている)
 *
 */
static W proc_duplicate(struct proc * source, struct proc * destination)
{
    W index;

    /* プロセスの仮想空間のコピー

     *    新しく仮想空間を生成し、コピー元の仮想空間からデータをコピーする。
     */

#ifdef FKDEBUG
    printk("fork: proc_duplicate: (%s file, %d line)\n", __FILE__, __LINE__);	/* */
#endif

    /* region のコピー */
    /* text */
    /* data+bss */
    /* heap */
    process_duplicate(source->proc_pid, destination->proc_pid);

    /* オープンファイルの情報のコピー
     */
    for (index = 0; index < MAX_OPEN; index++) {
	if (source->proc_open_file[index].f_inode != NULL) {
	    destination->proc_open_file[index] =
		source->proc_open_file[index];
	    destination->proc_open_file[index].f_inode->i_refcount++;
	}
    }

    /* copy of working directory */
    destination->proc_workdir = source->proc_workdir;
    destination->proc_workdir->i_refcount++;

    /* copy of uid/gid */
    destination->proc_uid = source->proc_uid;
    destination->proc_euid = source->proc_euid;
    destination->proc_gid = source->proc_gid;
    destination->proc_euid = source->proc_euid;

    /* copy of umask */
    destination->proc_umask = source->proc_umask;

    return (EOK);
}
