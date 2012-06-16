/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * 時間管理機能
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

#include "core.h"
#include "misc.h"
#include "arch.h"
#include "task.h"
#include "func.h"

/* 
 * 変数宣言
 */

static T_DALM almtable[MAX_ALARM];

SYSTIME system_time;
#define MS 1000

void init_time(UW seconds)
{
  SYSTIME time;
  UW TH, TM, TL;
  
  memset(almtable, 0, sizeof(T_DALM)*MAX_ALARM);

  TH = 0;
  TM = (seconds >> 16) * MS;
  TL = (seconds & 0x0FFFF) * MS;
  TM += TL >> 16;
  time.ltime = ((TM & 0x0FFFF) << 16) + (TL & 0x0FFFF);
  time.utime = TH + (TM >> 16);
  set_tim(&time);
}

/*
 * システムクロック設定
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
 * システムクロック参照
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
 * タスク遅延
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
 * アラームハンドラ定義
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
    memset(&(almtable[almno]), 0, sizeof(T_DALM));
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
    memcpy(&(almtable[almno].almtim), &(pk_dalm->almtim), sizeof(ALMTIME));
    set_timer(pk_dalm->almtim.ltime, (void (*)(VP))pk_dalm->almhdr,
	      pk_dalm->exinf);
  }
  return(E_OK);
}
