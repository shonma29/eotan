/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* process.c - POSIX 環境マネージャのプロセス管理処理部分
 *
 *
 *
 * $Log: process.c,v $
 * Revision 1.30  2000/06/23 09:18:13  naniwa
 * to support O_APPEND
 *
 * Revision 1.29  2000/05/06 03:52:27  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.28  2000/02/28 09:16:32  naniwa
 * minor fix
 *
 * Revision 1.27  2000/02/27 15:33:54  naniwa
 * to work as multi task OS
 *
 * Revision 1.26  2000/02/20 09:35:38  naniwa
 * minor fix
 *
 * Revision 1.25  2000/01/26 08:24:35  naniwa
 * to prevent memory leak
 *
 * Revision 1.24  2000/01/21 14:25:19  naniwa
 * to check fileid
 *
 * Revision 1.23  2000/01/18 14:41:08  naniwa
 * to close opend file at exit
 *
 * Revision 1.22  2000/01/15 15:29:30  naniwa
 * minor fix
 *
 * Revision 1.21  1999/07/23 14:39:03  naniwa
 * modified to implement exit
 *
 * Revision 1.20  1999/07/21 15:10:08  naniwa
 * modified to implement waitpid
 *
 * Revision 1.19  1999/03/24 04:52:10  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.18  1999/03/24 03:54:46  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.17  1999/03/21 14:47:12  monaka
 * Function proc_get_ppid added.
 *
 * Revision 1.16  1998/02/24 14:20:07  night
 * プロセステーブルを初期化するときに、プロセス 0 だけは、
 * 使用中として確保するように変更した。
 * (プロセス 0 は、init プログラムにて使用する)
 *
 * Revision 1.15  1998/02/23 14:49:20  night
 * proc_vm_dump 関数の追加。
 *
 * Revision 1.14  1998/02/16 14:26:49  night
 * proc_set_info() を実行するときにプロセスの状態をチェックしていた
 * 処理を削除した。
 * proc_set_info() で対象となるプロセスは、生成していない状態の場合も
 * あるので、チェックする必要はない。
 *
 * Revision 1.13  1997/10/24 13:59:50  night
 * 変数 free_proc、run_proc の追加。
 *
 * Revision 1.12  1997/10/23 14:32:33  night
 * exec システムコール関係の処理の更新
 *
 * Revision 1.11  1997/10/22 14:56:07  night
 * proc_set_gid () を作成した。
 *
 * Revision 1.10  1997/08/31 13:34:33  night
 * プロセス情報の設定時、work directory を強制的に rootfile の値に設定するように
 * した(以前は、NULL に設定していた)。
 * 以下の関数追加。
 *
 *  proc_set_euid (W procid, W uid)
 *  proc_set_egid (W procid, W gid)
 *
 * Revision 1.9  1997/07/07 12:17:41  night
 * proc_get_euid と proc_get_egid を追加。
 *
 * Revision 1.8  1997/05/12 14:31:51  night
 * misc システムコールに M_PROC_DUMP コマンドを追加。
 *
 * Revision 1.7  1997/05/08 15:11:30  night
 * プロセスの情報を設定する機能の追加。
 * (syscall misc の proc_set_info コマンド)
 *
 * Revision 1.6  1997/05/06 12:47:50  night
 * set_procinfo システムコールの追加。
 *
 * Revision 1.5  1996/11/20  12:12:28  night
 * proc_set_file() において、ファイル記述子で指定されたファイルインデック
 * スがすでに使われているかどうかのチェックを追加。
 *
 * Revision 1.4  1996/11/18  13:44:04  night
 * 関数 proc_get_file() を追加。
 *
 * Revision 1.3  1996/11/17  16:48:30  night
 * init_process() の中身を作成した。
 * proc_get_umask(), proc_set_umask() を作成した。
 *
 * Revision 1.2  1996/11/14  13:17:38  night
 * プロセス構造体の情報を取得する関数を追加。
 *
 * Revision 1.1  1996/11/05  15:13:46  night
 * 最初の登録
 *
 */

#include <fcntl.h>
#include <local.h>
#include <string.h>
#include <boot/init.h>
#include <nerve/kcall.h>
#include "fs.h"
#include "../../lib/libserv/libmm.h"

struct proc proc_table[MAX_PROCESS];
static struct proc *free_proc, *tail_proc;



/* init_process
 *
 */
W init_process(void)
{
    W i;

    for (i = 0; i < MAX_PROCESS; i++) {
	memset((B*)&proc_table[i], 0, sizeof(struct proc));
/* if set explicitly
	proc_table[i].proc_status = PS_DORMANT;
*/
	proc_table[i].proc_next = &proc_table[i + 1];
	proc_table[i].proc_pid = i;
    }
    i--;
    proc_table[i].proc_next = NULL;

    free_proc = &proc_table[INIT_PID + 1];
    tail_proc = &proc_table[i];

    proc_table[INIT_PID].proc_status = PS_SLEEP;	/* プロセス 0 については、最初に確保しておく */
    proc_table[INIT_PID].proc_next = NULL;
    return (E_OK);
}

W set_local(ID pid, ID tskid)
{
    W error_no;
    thread_local_t local_data;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = vmap(pid, (thread_local_t*)LOCAL_ADDR, sizeof(thread_local_t),
    		true);
    if (error_no)
	return error_no;

    memset(&local_data, 0, sizeof(local_data));
    local_data.thread_id = tskid;
    local_data.process_id = pid;
    strcpy((B*)local_data.cwd, "/");
    local_data.cwd_length = 1;

    error_no = kcall->region_put(tskid, (thread_local_t*)LOCAL_ADDR,
		     sizeof(thread_local_t), &local_data);
    if (error_no)
	return error_no;
    return (EOK);
}

/* プロセスを終了する
 */
W proc_exit(W procid)
{
    int i;
    struct proc *procp;
    void sfs_close_device();

    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    /* プロセス情報の初期化 */
    /* proc_pid == 0 でプロセスは未使用とはならない
       proc_table[procid].proc_pid = 0;
     */
    proc_table[procid].proc_status = PS_DORMANT;

    /* プロセスの情報の解放
     */
    procp = &proc_table[procid];
    /* working directory の開放 */
    if (procp->proc_workdir != NULL) {
	fs_close_file(procp->proc_workdir);
	procp->proc_workdir = NULL;
    }

    /* open されているファイルの close */
    for (i = 0; i < MAX_OPEN; ++i) {
	if (procp->proc_open_file[i].f_inode != NULL) {
	    if (procp->proc_open_file[i].f_inode->i_mode & S_IFCHR) {
		/* スペシャルファイルだった */
		/* デバイスに DEV_CLS メッセージを発信 */
		sfs_close_device(procp->proc_open_file[i].f_inode->i_dev);
	    }
	    fs_close_file(procp->proc_open_file[i].f_inode);
	    procp->proc_open_file[i].f_inode = NULL;
	}
    }

    proc_dealloc_proc(procid);

    return (EOK);
}


W proc_get_procp(W procid, struct proc ** procp)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *procp = &proc_table[procid];
    return (EOK);
}



W proc_get_pid(W procid, W * pid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *pid = proc_table[procid].proc_pid;
    return (EOK);
}



W proc_get_ppid(W procid, W * ppid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *ppid = proc_table[procid].proc_ppid;
    return (EOK);
}



W proc_get_uid(W procid, W * uid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *uid = proc_table[procid].proc_uid;
    return (EOK);
}


W proc_get_gid(W procid, W * gid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *gid = proc_table[procid].proc_gid;
    return (EOK);
}


W proc_set_gid(W procid, W gid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    proc_table[procid].proc_gid = gid;
    return (EOK);
}


W proc_get_euid(W procid, W * uid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *uid = proc_table[procid].proc_euid;
    return (EOK);
}



W proc_set_euid(W procid, W uid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    proc_table[procid].proc_euid = uid;
    return (EOK);
}



W proc_get_egid(W procid, W * gid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *gid = proc_table[procid].proc_egid;
    return (EOK);
}



W proc_set_egid(W procid, W gid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    proc_table[procid].proc_egid = gid;
    return (EOK);
}


W proc_alloc_fileid(W procid, W * retval)
{
    W i;

    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    for (i = 0; i < MAX_OPEN; i++) {
	if (proc_table[procid].proc_open_file[i].f_inode == NULL) {
	    *retval = i;
	    memset((B*)&(proc_table[procid].proc_open_file[i]), 0,
		  sizeof(struct file));
	    return (EOK);
	}
    }
    return (ENOMEM);
}


W proc_set_file(W procid, W fileid, W flag, struct inode * ip)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    if ((fileid < 0) || (fileid >= MAX_OPEN)) {
	return (EINVAL);
    }

    if (proc_table[procid].proc_open_file[fileid].f_inode != NULL) {
	return (EBADF);
    }

    proc_table[procid].proc_open_file[fileid].f_inode = ip;
    if ((flag & O_APPEND) != 0) {
	proc_table[procid].proc_open_file[fileid].f_offset = ip->i_size;
    } else {
	proc_table[procid].proc_open_file[fileid].f_offset = 0;
    }
    proc_table[procid].proc_open_file[fileid].f_omode = flag & 0x03;
    return (EOK);
}


W proc_get_file(W procid, W fileid, struct file ** fp)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    if ((fileid < 0) || (fileid >= MAX_OPEN)) {
	return (EINVAL);
    }

    *fp = &(proc_table[procid].proc_open_file[fileid]);
    return (EOK);
}


W proc_get_cwd(W procid, struct inode ** cwd)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *cwd = proc_table[procid].proc_workdir;
    if (*cwd == NULL) {
	return (ESRCH);
    }

    return (EOK);
}

W proc_set_cwd(W procid, struct inode * cwd)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    if (cwd == (struct inode *) 0) {
	return (EINVAL);
    }

    proc_table[procid].proc_workdir = cwd;
    return (EOK);
}


W proc_set_umask(W procid, W umask)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    proc_table[procid].proc_umask = umask;
    return (EOK);
}


W proc_get_umask(W procid, W * umask)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *umask = proc_table[procid].proc_umask;
    return (EOK);
}


struct vm_tree *proc_get_vmtree(W procid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (NULL);
    }

    return (proc_table[procid].vm_tree);
}




/* proc_new_proc - 新しいプロセスを生成する
 *
 *
 */
W proc_alloc_proc(struct proc ** procp)
{
    W pid;

    if (free_proc == NULL) {
	return (ENOMEM);
    }

    *procp = free_proc;
    free_proc = (*procp)->proc_next;
    if (free_proc == NULL) {
	tail_proc = NULL;
    }

    pid = (*procp)->proc_pid;
    memset((B*)(*procp), 0, sizeof(struct proc));
/* if set explicitly
    (*procp)->proc_status = PS_DORMANT;
    (*procp)->proc_next = NULL;
*/
    (*procp)->proc_pid = pid;

    return (EOK);
}

void proc_dealloc_proc(W procid)
{
    proc_table[procid].proc_status = PS_DORMANT;
    proc_table[procid].proc_next = NULL;

    /* フリー・プロセス・リストの最後に登録 */
    if (tail_proc) {
	tail_proc->proc_next = &proc_table[procid];
    }

    tail_proc = &proc_table[procid];
}
