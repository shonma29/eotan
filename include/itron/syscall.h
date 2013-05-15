
#ifndef __ITRON_SYSCALL_H__
#define __ITRON_SYSCALL_H__

/* システムコール関数の定義 */
/* --------------------------------------------------------- */
/* タスク管理機能                                            */
/* --------------------------------------------------------- */

#include "types.h"
#include "struct.h"

extern ER_ID	acre_tsk (T_CTSK *pk_ctsk);
extern ER	del_tsk (ID tskid);
extern ER	sta_tsk (ID tskid, INT stacd);
extern void	ext_tsk (void);
extern void	exd_tsk (void);
extern ER	ter_tsk (ID tskid);
extern ER	rel_wai (ID tskid);
extern ER	get_tid (ID *p_tskid);

extern ER	sus_tsk (ID taskid);
extern ER	rsm_tsk (ID taskid);

/* --------------------------------------------------------- */
/* Interrupt                                                 */
/* --------------------------------------------------------- */
extern ER def_int(W intno, T_DINT *pk_dint);

/* --------------------------------------------------------- */
/* 時間管理                                                    */
/* --------------------------------------------------------- */
extern ER set_tim(SYSTIME *pk_tim);
extern ER get_tim(SYSTIME *pk_tim);
extern ER dly_tsk(DLYTIME dlytim);

/* --------------------------------------------------------- */
/* MISC                                                      */
/* --------------------------------------------------------- */
extern ER vsys_inf(VP buf);
extern ER vcpy_stk(ID src, W esp, ID dst);
extern ER vset_ctx (ID tid, W eip, B *stackp, W stsize);

#endif /*__ITRON_SYSCALL_H__*/
