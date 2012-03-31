/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * ���ִ�����ǽ
 */
/*
 * $Log: time.c,v $
 * Revision 1.4  2000/04/03 14:34:48  naniwa
 * to call timer handler in task
 *
 * Revision 1.3  2000/02/27 15:30:51  naniwa
 * to work as multi task OS
 *
 * Revision 1.2  2000/02/06 09:10:58  naniwa
 * minor fix
 *
 * Revision 1.1  1999/11/14 14:53:38  naniwa
 * add time management function
 *
 */

#include "itron.h"
#include "errno.h"
#include "misc.h"
#include "ibmpc.h"
#include "time.h"
#include "task.h"
#include "func.h"

/* 
 * �ѿ����
 */

static T_CYCHANDLER cyctable[MAX_CYCLIC];
static T_ALMHANDLER almtable[MAX_ALARM];

SYSTIME system_time;
#define MS 1000

void init_time(UW seconds)
{
  SYSTIME time;
  UW TH, TM, TL;
  
  bzero(cyctable, sizeof(T_CYCHANDLER)*MAX_CYCLIC);
  bzero(almtable, sizeof(T_ALMHANDLER)*MAX_ALARM);

  TH = 0;
  TM = (seconds >> 16) * MS;
  TL = (seconds & 0x0FFFF) * MS;
  TM += TL >> 16;
  time.ltime = ((TM & 0x0FFFF) << 16) + (TL & 0x0FFFF);
  time.utime = TH + (TM >> 16);
  set_tim(&time);
}

/*
 * �����ƥ९��å�����
 */
ER set_tim(SYSTIME *pk_tim)
{
  if (pk_tim == NULL) {
    return (E_PAR);
  }
  system_time.utime = pk_tim->utime;
  system_time.ltime = pk_tim->ltime;
  return (E_OK);
}

/*
 * �����ƥ९��å�����
 */
ER get_tim(SYSTIME *pk_tim)
{
  if (pk_tim == NULL) {
    return (E_PAR);
  }
  pk_tim->utime = system_time.utime;
  pk_tim->ltime = system_time.ltime;
  return (E_OK);
}

/*
 * �������ٱ�
 */

static void
dly_func (VP p)
{
  T_TCB *taskp;

#ifdef CALL_HANDLER_IN_TASK
  dis_dsp();
#else
  dis_int();
#endif
  taskp = (T_TCB *)p;
  taskp->slp_err = E_OK;
  taskp->tskwait.time_wait = 0;
#ifdef CALL_HANDLER_IN_TASK
  ena_dsp();
#else
  ena_int();
#endif
  wup_tsk (taskp->tskid);
}

ER dly_tsk(DLYTIME dlytim)
{
  if (dlytim < 0) return(E_PAR);
  else if (! dlytim) return(E_OK);
  set_timer (dlytim, (void (*)(VP))dly_func, run_task);
  run_task->tskwait.time_wait = 1;
  slp_tsk ();
  unset_timer ((void (*)(VP))dly_func, run_task);
  if (run_task->slp_err)
    return (run_task->slp_err);
  return(E_OK);
}

/*
 * ������ư�ϥ�ɥ����
 */

ER def_cyc(HNO cycno, T_DCYC *pk_dcyc)
{
  return(E_NOSPT);
}

/*
 * ������ư�ϥ�ɥ��������
 */

ER act_cyc(HNO cycno, UINT cycact)
{
  return(E_NOSPT);
}

/*
 * ������ư�ϥ�ɥ���ֻ���
 */

ER ref_cyc(T_RCYC *pk_rcyc, HNO cycno)
{
  return(E_NOSPT);
}

/*
 * ���顼��ϥ�ɥ����
 */

ER def_alm(HNO almno, T_DALM *pk_dalm)
{
  if (almno < 0) return(E_PAR);
  else if (almno >= MAX_ALARM) return(E_PAR);
  if (pk_dalm == NULL) return (E_PAR);
  if (pk_dalm == (T_DALM *) NADR) {
    if (almtable[almno].almhdr != NULL) {
      unset_timer((void (*)(VP))almtable[almno].almhdr, almtable[almno].exinf);
    }
    bzero(&(almtable[almno]), sizeof(T_ALMHANDLER));
  }
  else {
    if (pk_dalm->tmmode == TTM_ABS) {
      return(E_NOSPT);
    }
    if (pk_dalm->almtim.utime > 0) {
      return(E_NOSPT);
    }
    if (almtable[almno].almhdr != NULL) {
      unset_timer((void (*)(VP))almtable[almno].almhdr, almtable[almno].exinf);
    }

    almtable[almno].exinf = pk_dalm->exinf;
    almtable[almno].almhdr = pk_dalm->almhdr;
    almtable[almno].almatr = pk_dalm->almatr;
    almtable[almno].tmmode = pk_dalm->tmmode;
    bcopy(&(pk_dalm->almtim), &(almtable[almno].almtim), sizeof(ALMTIME));
    set_timer(pk_dalm->almtim.ltime, (void (*)(VP))pk_dalm->almhdr,
	      pk_dalm->exinf);
  }
  return(E_OK);
}

/*
 * ���顼��ϥ�ɥ���ֻ���
 */

ER ref_alm(T_RALM *pk_ralm, HNO almno)
{
  if (almno < 0) return(E_PAR);
  else if (almno >= MAX_ALARM) return(E_PAR);
  if (pk_ralm == NULL) return(E_PAR);
  if (almtable[almno].almhdr == NULL) return(E_NOEXS);
  pk_ralm->exinf = almtable[almno].exinf;
  pk_ralm->lfttim.utime = 0;
  pk_ralm->lfttim.ltime = left_time((void (*)(VP))almtable[almno].almhdr,
				    almtable[almno].exinf);
  return(E_OK);
}
