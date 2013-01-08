/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001,2002 Tomohide Naniwa

*/
/*
 * $Log: kill.c,v $
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "thread.h"
#include "posix.h"

W psc_kill_f(RDVNO rdvno, struct posix_request *req)
{
    struct proc *myprocp, *procp;
    struct posix_request preq;
    W mypid, wpid, exst;
    W i;
    ER errno;

    /* req->caller が task 1 の場合は，返事のメッセージを送らない */

    mypid = req->param.par_kill.pid;
    errno = proc_get_procp(mypid, &myprocp);
    if (errno) {
	return errno;
	if (req->caller != KERNEL_TASK) {
	    put_response(rdvno, req, EP_SRCH, 0, 0, 0);
	}
    }
    myprocp->proc_exst = (-1);	/* 強制終了時のステータスは (-1) で良いか? */

    errno = proc_get_procp(myprocp->proc_ppid, &procp);
    if (errno) {
	return errno;
	if (req->caller != KERNEL_TASK) {
	    put_response(rdvno, req, EP_SRCH, 0, 0, 0);
	}
    }
    wpid = procp->proc_wpid;
    if (procp->proc_status == PS_WAIT &&
	(wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
	/* 親プロセスが自分を WAIT していればメッセージ送信 */
	procp->proc_status = PS_RUN;
	preq.operation = PSC_WAITPID;
	exst = (myprocp->proc_exst << 8);
	put_response(rdvno, &preq, EP_OK, mypid, exst, 0);

	/* エントリーの開放 */
	proc_exit(mypid);
    } else {
	/* そうでなければ，ZOMBIE 状態に */
	myprocp->proc_status = PS_ZOMBIE;
    }

    /* 子プロセスの親を INIT に変更 */
    for (i = 1; i < MAX_PROCESS; ++i) {
	proc_get_procp(i, &procp);
	if (procp->proc_status == PS_DORMANT)
	    continue;
	if (procp->proc_ppid != mypid)
	    continue;
	procp->proc_ppid = 0;	/* INIT プロセスの pid は 0 */

	/* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
    }

    /* POSIX の vmtree のみを開放．仮想メモリーは del_tsk で開放 */
    destroy_proc_memory(myprocp, 0);

    /* メインタスクの強制終了 */
    ter_tsk(myprocp->proc_maintask);
    del_tsk(myprocp->proc_maintask);

    if ((req->caller != KERNEL_TASK) &&
	(req->caller != myprocp->proc_maintask)) {
	put_response(rdvno, req, EP_OK, 0, 0, 0);
    }
    return (SUCCESS);
}
