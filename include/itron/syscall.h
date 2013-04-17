
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

extern ER def_int(W intno, T_DINT *pk_dint);

/* --------------------------------------------------------- */
/* イベントフラグ用関数                                      */
/* --------------------------------------------------------- */
extern ER_ID	acre_flg (T_CFLG *pk_cflg);
extern ER	del_flg (ID flgid);
extern ER	set_flg (ID flgid, UINT setptn);
extern ER	clr_flg (ID flgid, UINT clrptn);
extern ER	wai_flg (UINT *p_flgptn, ID flgid, UINT waiptn, UINT wfmode);

/* --------------------------------------------------------- */
/* Interrupt                                                 */
/* --------------------------------------------------------- */

/* --------------------------------------------------------- */
/* Memory                                                    */
/* --------------------------------------------------------- */
extern ER vcre_reg (ID id, ID rid, VP start, W min, W max, UW perm, FP handle);
extern ER vdel_reg (ID id, ID rid);
extern ER vdup_reg (ID src, ID dst);
extern ER vsts_reg (ID id, ID rid, VP stat);
extern ER vmap_reg (ID id, VP start, UW size, W accmode);
extern ER vunm_reg (ID id, VP addr, UW size);
extern ER vget_reg (ID id, VP start, UW size, VP buf);
extern ER vput_reg (ID id, VP start, UW size, VP buf);
extern ER vget_phs (ID id, VP addr, UW *paddr);

/* --------------------------------------------------------- */
/* 時間管理                                                    */
/* --------------------------------------------------------- */
extern ER set_tim(SYSTIME *pk_tim);
extern ER get_tim(SYSTIME *pk_tim);
extern ER dly_tsk(DLYTIME dlytim);
extern ER def_alm(HNO almo, T_DALM *pk_dalm);

/* --------------------------------------------------------- */
/* MISC                                                      */
/* --------------------------------------------------------- */
extern ER vsys_inf (W func, W sub, VP argp);
extern ER vcpy_stk(ID src, W esp, W ebp, W ebx, W ecx, W edx, W esi , W edi, ID dst);
extern ER vset_ctx (ID tid, W eip, B *stackp, W stsize);
extern ER vuse_fpu (ID tid);

#endif /*__ITRON_SYSCALL_H__*/
