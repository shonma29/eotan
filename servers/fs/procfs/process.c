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
#include <major.h>
#include <string.h>
#include <boot/init.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include "api.h"
#include "devfs/devfs.h"
#include "procfs/process.h"
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

/* プロセスを終了する
 */
W proc_exit(W procid)
{
    int i;
    struct proc *procp;

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
    if (procp->session.cwd != NULL) {
	vnodes_remove(procp->session.cwd);
	procp->session.cwd = NULL;
    }

    /* open されているファイルの close */
    for (i = 0; i < MAX_FILE; ++i) {
	if (procp->session.files[i].f_vnode != NULL) {
	    vnodes_remove(procp->session.files[i].f_vnode);
	    procp->session.files[i].f_vnode = NULL;
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



W proc_get_permission(W procid, struct permission * p)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *p = proc_table[procid].session.permission;
    return (EOK);
}


W proc_get_status(W procid)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (-1);
    }

    return proc_table[procid].proc_status;
}


W proc_alloc_fileid(W procid, W * retval)
{
    W i;

    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    for (i = 0; i < MAX_FILE; i++) {
	if (proc_table[procid].session.files[i].f_vnode == NULL) {
	    *retval = i;
	    memset((B*)&(proc_table[procid].session.files[i]), 0,
		  sizeof(struct file));
	    return (EOK);
	}
    }
    return (ENOMEM);
}


W proc_set_file(W procid, W fileid, W flag, vnode_t * ip)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    if ((fileid < 0) || (fileid >= MAX_FILE)) {
	return (EINVAL);
    }

    if (proc_table[procid].session.files[fileid].f_vnode != NULL) {
	return (EBADF);
    }

    proc_table[procid].session.files[fileid].f_vnode = ip;
    if ((flag & O_APPEND) != 0) {
	proc_table[procid].session.files[fileid].f_offset = ip->size;
    } else {
	proc_table[procid].session.files[fileid].f_offset = 0;
    }
    proc_table[procid].session.files[fileid].f_flag = flag & 0x03;
    return (EOK);
}


W proc_get_file(W procid, W fileid, struct file ** fp)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    if ((fileid < 0) || (fileid >= MAX_FILE)) {
	return (EINVAL);
    }

    *fp = &(proc_table[procid].session.files[fileid]);
    return (EOK);
}


W proc_get_cwd(W procid, vnode_t ** cwd)
{
    if ((procid < INIT_PID) || (procid >= MAX_PROCESS)) {
	return (EINVAL);
    }

    *cwd = proc_table[procid].session.cwd;
    if (*cwd == NULL) {
	return (ESRCH);
    }

    return (EOK);
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

/* file discriptor 0, 1, 2 の設定
 */
W open_special_devices(struct proc * procp)
{
    vnode_t *ip;
    device_info_t *p;

    p = device_find(get_device_id(DEVICE_MAJOR_CONS, 0));
    if (p) {
	/* 標準入力の設定 */
	procp->session.files[0].f_vnode = ip = vnodes_create();
	procp->session.files[0].f_offset = 0;
	procp->session.files[0].f_flag = O_RDONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->mode = S_IFCHR;
	ip->dev = p->id;
	ip->fs = &devfs;
	ip->index = -1;
	ip->size = p->size;
	ip->nblock = 0;
	ip->refer_count = 1;
	vnodes_append(ip);

	/* 標準出力の設定 */
	procp->session.files[1].f_vnode = ip = vnodes_create();
	procp->session.files[1].f_offset = 0;
	procp->session.files[1].f_flag = O_WRONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->mode = S_IFCHR;
	ip->dev = p->id;
	ip->fs = &devfs;
	ip->index = -2;
	ip->size = p->size;
	ip->nblock = 0;
	ip->refer_count = 1;
	vnodes_append(ip);

	/* 標準エラー出力の設定 */
	procp->session.files[2].f_vnode = ip = vnodes_create();
	procp->session.files[2].f_offset = 0;
	procp->session.files[2].f_flag = O_WRONLY;
	if (ip == NULL) {
	    return (ENOMEM);
	}
	ip->mode = S_IFCHR;
	ip->dev = p->id;
	ip->fs = &devfs;
	ip->index = -3;
	ip->size = p->size;
	ip->nblock = 0;
	ip->refer_count = 1;
	vnodes_append(ip);
    }

    return (EOK);
}
