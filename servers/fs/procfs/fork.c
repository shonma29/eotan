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

#include <string.h>
#include "api.h"
#include "procfs/process.h"
#include "../../lib/libserv/libmm.h"

static W proc_duplicate(struct proc * source, struct proc * destination);


/* fork - プロセスの fork を行う
 *
 *
 */
W proc_fork(struct proc *parent, struct proc *child)
{
    W error_no;

    /* プロセス情報のコピー */
    /* 管理情報 (プロセスおよびファイル)の更新 */
    error_no = proc_duplicate(parent, child);
    if (error_no) {
	return (error_no);
    }

    child->proc_status = PS_RUN;
    strncpy(child->proc_name, parent->proc_name, PROC_NAME_LEN - 1);
    child->proc_name[PROC_NAME_LEN - 1] = '\0';

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

    /* region のコピー */
    /* text */
    /* data+bss */
    /* heap */
    process_duplicate(source->proc_pid, destination->proc_pid);

    /* オープンファイルの情報のコピー
     */
    for (index = 0; index < MAX_FILE; index++) {
	struct file *srcfile = session_find_desc(source->session, index);
	if (srcfile) {
	    struct file *destfile;
	    //TODO check error
	    session_create_desc(&destfile, destination->session,
		    index);
	    destfile->f_vnode = srcfile->f_vnode;
	    destfile->f_flag = srcfile->f_flag;
	    destfile->f_count = srcfile->f_count;
	    destfile->f_offset = srcfile->f_offset;
	}
    }

    /* copy of working directory */
    destination->session->cwd = source->session->cwd;
//TODO is not needed?
    destination->session->cwd->refer_count++;

    /* copy of uid/gid */
    destination->session->permission = source->session->permission;

    return (EOK);
}
