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

#include <string.h>
#include "core.h"
#include "misc.h"
#include "thread.h"
#include "func.h"
#include "sync.h"
#include "arch/archfunc.h"

/* 
 * 変数宣言
 */

static T_DALM almtable[MAX_ALARM];

SYSTIME system_time;
#define MS 1000

void time_initialize(UW seconds)
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
  time_set(&time);
}

/*
 * システムクロック設定
 */
ER time_set(SYSTIME *pk_tim)
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
ER time_get(SYSTIME *pk_tim)
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

  enter_serialize();
  taskp = (T_TCB *)p;
  taskp->wait.result = E_OK;
  leave_serialize();

  release(taskp);
}

ER thread_delay(DLYTIME dlytim)
{
  if (dlytim < 0) return(E_PAR);
  else if (! dlytim) return(E_OK);
  set_timer (dlytim, (void (*)(VP))dly_func, run_task);
  run_task->wait.type = wait_dly;
  wait(run_task);
  unset_timer ((void (*)(VP))dly_func, run_task);
  return (run_task->wait.result);
}

/*
 * アラームハンドラ定義
 */

ER alarm_create(HNO almno, T_DALM *pk_dalm)
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
