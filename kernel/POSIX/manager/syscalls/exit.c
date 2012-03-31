/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/*
 * $Log: exit.c,v $
 * Revision 1.2  1999/07/23 14:41:10  naniwa
 * implemented
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

/* psc_exit_f - �ץ�����λ������
 */
W
psc_exit_f (struct posix_request *req)
{
  struct proc *myprocp, *procp;
  struct posix_request preq;
  W mypid, wpid, exst;
  W i;
  ER errno;
  ID tskid;

  mypid = req->procid;
  errno = proc_get_procp(mypid, &myprocp);
  if (errno) {
    put_response (req, EP_SRCH, 0, 0, 0);
    /* ��å������θƤӽФ����˥��顼���֤��Ƥ�����Ǥ��ʤ�����
       �������� exd_tsk �ǽ�λ���� */
    return errno;
  }

  myprocp->proc_exst = req->param.par_exit.evalue;

  errno = proc_get_procp(myprocp->proc_ppid, &procp);
  if (errno) {
    put_response (req, EP_SRCH, 0, 0, 0);
    /* ��å������θƤӽФ����˥��顼���֤��Ƥ�����Ǥ��ʤ�����
       �������� exd_tsk �ǽ�λ���� */
    return errno;
  }

  wpid = procp->proc_wpid;
  if (procp->proc_status == PS_WAIT &&
      (wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
    /* �ƥץ�������ʬ�� WAIT ���Ƥ���Х�å��������� */
    procp->proc_status = PS_RUN;
    preq.receive_port = procp->proc_rvpt;
    preq.operation = PSC_WAITPID;
    exst = (myprocp->proc_exst << 8);
    put_response (&preq, EP_OK, mypid, exst, 0);

    /* ����ȥ꡼�γ��� */
    proc_exit(mypid);
  }
  else {
    /* �����Ǥʤ���С�ZOMBIE ���֤� */
    myprocp->proc_status = PS_ZOMBIE;
  }

  /* �ҥץ����οƤ� INIT ���ѹ� */
  for(i = 1; i < MAX_PROCESS; ++i) {
    proc_get_procp(i, &procp);
    if (procp->proc_status == PS_DORMANT) continue;
    if (procp->proc_ppid != mypid) continue;
    procp->proc_ppid = 0; /* INIT �ץ����� pid �� 0 */
    
    /* �ҥץ����� ZOMBIE �� INIT �� wait ���Ƥ���� ���ꥢ����? */
  }

  /* POSIX �� vmtree �Τߤ��������ۥ��꡼�� exd_tsk �ǳ��� */
  destroy_proc_memory (myprocp, 0);

  /* region �γ���, �ºݤˤ�ɬ��̵������ */
  tskid = req->caller,
  vdel_reg(tskid, TEXT_REGION); /* text */
  vdel_reg(tskid, DATA_REGION); /* data+bss */
  vdel_reg(tskid, HEAP_REGION); /* heap */
#ifdef notdef
  vdel_reg(tskid, STACK_REGION); /* stack */
#endif

  put_response (req, EP_OK, 0, 0, 0);
  return (SUCCESS);
}  
