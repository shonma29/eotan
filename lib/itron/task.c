/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* sys_task.c -- タスク関係のシステムコール
 *
 * $Id: sys_task.c,v 1.4 2000/04/03 14:37:35 naniwa Exp $
 */

#include <core.h>
#include <api.h>
#include "call_kernel.h"


/* cre_tsk  --- タスクの生成
 *
 * 引数：
 *
 * 返り値：
 *
 */
ER
cre_tsk (ID tskid, T_CTSK *pk_ctsk)
{
  return ncall(SYS_CRE_TSK, tskid, pk_ctsk);
}


/* ext_tsk  --- 自タスク終了
*/
void
ext_tsk (void)
{
  ncall(SYS_EXT_TSK);
}

/* exd_tsk  --- 自タスク終了と削除
*/
void
exd_tsk (void)
{
  ncall(SYS_EXD_TSK);
}

/* chg_pri  --- プライオリティの変更
*/
ER
chg_pri (ID tskid, PRI tskpri)
{
  return ncall(SYS_CHG_PRI, tskid, tskpri);
}

/* rel_wai --- 待ち状態の解除
 */
ER
rel_wai (ID taskid)
{
  return ncall(SYS_REL_WAI, taskid);
}

/* get_tid  --- 自タスクのタスク ID 参照
*/
ER
get_tid (ID *rid)
{
  return ncall(SYS_GET_TID, rid);
}

/* rsm_tsk  --- 強制待ち状態のタスクから待ち状態を解除
*/
ER
rsm_tsk (ID taskid)
{
  return ncall(SYS_RSM_TSK, taskid);
}

/* sta_tsk  --- タスクの起動
*/
ER
sta_tsk (ID taskid, INT stacd)
{
  return ncall(SYS_STA_TSK, taskid, stacd);
}

/* sus_tsk  --- 指定したタスクを強制待ち状態に移行
*/
ER
sus_tsk (ID taskid)
{
  return ncall(SYS_SUS_TSK, taskid);
}

/* ter_tsk  --- 他タスク強制終了
*/
ER
ter_tsk (ID tskid)
{
  return ncall(SYS_TER_TSK, tskid);
}

/* del_tsk --- 他タスク削除
 */
ER
del_tsk (ID tskid)
{
  return ncall(SYS_DEL_TSK, tskid);
}
