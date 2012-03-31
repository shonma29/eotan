/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

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

#include "h/task.h"
#include "posix.h"
#include "h/lowlib.h"

W psc_kill_f(struct posix_request *req)
{
    struct proc *myprocp, *procp;
    struct posix_request preq;
    W mypid, wpid, exst;
    W i;
    ER errno;
    struct lowlib_data lowlib_data;

    /* req->caller �� task 1 �ξ��ϡ��ֻ��Υ�å�����������ʤ� */

    mypid = req->param.par_kill.pid;
    errno = proc_get_procp(mypid, &myprocp);
    if (errno) {
	return errno;
	if (req->caller != KERNEL_TASK) {
	    put_response(req, EP_SRCH, 0, 0, 0);
	}
    }
    myprocp->proc_exst = (-1);	/* ������λ���Υ��ơ������� (-1) ���ɤ���? */

    errno = proc_get_procp(myprocp->proc_ppid, &procp);
    if (errno) {
	return errno;
	if (req->caller != KERNEL_TASK) {
	    put_response(req, EP_SRCH, 0, 0, 0);
	}
    }
    wpid = procp->proc_wpid;
    if (procp->proc_status == PS_WAIT &&
	(wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
	/* �ƥץ�������ʬ�� WAIT ���Ƥ���Х�å��������� */
	procp->proc_status = PS_RUN;
	preq.receive_port = procp->proc_rvpt;
	preq.operation = PSC_WAITPID;
	exst = (myprocp->proc_exst << 8);
	put_response(&preq, EP_OK, mypid, exst, 0);

	/* ����ȥ꡼�γ��� */
	proc_exit(mypid);
    } else {
	/* �����Ǥʤ���С�ZOMBIE ���֤� */
	myprocp->proc_status = PS_ZOMBIE;
    }

    /* �ҥץ����οƤ� INIT ���ѹ� */
    for (i = 1; i < MAX_PROCESS; ++i) {
	proc_get_procp(i, &procp);
	if (procp->proc_status == PS_DORMANT)
	    continue;
	if (procp->proc_ppid != mypid)
	    continue;
	procp->proc_ppid = 0;	/* INIT �ץ����� pid �� 0 */

	/* �ҥץ����� ZOMBIE �� INIT �� wait ���Ƥ���� ���ꥢ����? */
    }

    /* POSIX �� vmtree �Τߤ��������ۥ��꡼�� del_tsk �ǳ��� */
    destroy_proc_memory(myprocp, 0);

    /* �ᥤ�󥿥����� mbf �γ��� */
    errno = vget_reg(myprocp->proc_maintask, LOWLIB_DATA,
		     sizeof(struct lowlib_data), &lowlib_data);
    del_mbf(lowlib_data.recv_port);

    /* �ᥤ�󥿥����ζ�����λ */
    ter_tsk(myprocp->proc_maintask);
    del_tsk(myprocp->proc_maintask);

    if ((req->caller != KERNEL_TASK) &&
	(req->caller != myprocp->proc_maintask)) {
	put_response(req, EP_OK, 0, 0, 0);
    }
    return (SUCCESS);
}
