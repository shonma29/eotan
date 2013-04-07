
#ifndef __ITRON_SYSCALL_H__
#define __ITRON_SYSCALL_H__

/* システムコール関数の定義 */
/* --------------------------------------------------------- */
/* タスク管理機能                                            */
/* --------------------------------------------------------- */

#include "types.h"
#include "struct.h"

/* システムコール番号の定義 
 */
#define SYS_CRE_TSK	(1)
#define	SYS_DEL_TSK	(2)
#define	SYS_STA_TSK	(3)
#define	SYS_EXT_TSK	(4)
#define	SYS_EXD_TSK	(5)
#define	SYS_TER_TSK	(6)
#define	SYS_CHG_PRI	(7)
#define	SYS_REL_WAI	(8)
#define	SYS_GET_TID	(9)

#define	SYS_SUS_TSK	(10)
#define	SYS_RSM_TSK	(11)

#define SYS_ACRE_FLG	(12)
#define SYS_DEL_FLG   	(13)
#define SYS_SET_FLG   	(14)
#define SYS_CLR_FLG  	(15)
#define SYS_WAI_FLG   	(16)

#define SYS_SET_TIM	(17)
#define SYS_GET_TIM	(18)
#define SYS_DLY_TSK	(19)

#define SYS_DEF_ALM	(20)

#define SYS_DEF_INT	(21)

#define SYS_VSYS_INF	(22)
#define SYS_DBG_PUTS	(23)

#define SYS_VCRE_REG	(24)
#define SYS_VDEL_REG	(25)
#define SYS_VMAP_REG	(26)
#define SYS_VUNM_REG	(27)
#define SYS_VDUP_REG	(28)
#define SYS_VPUT_REG	(29)
#define SYS_VGET_REG	(30)
#define SYS_VSTS_REG	(31)

#define SYS_VGET_PHS	(32)

#define SYS_VSYS_MSC	(33)
#define SYS_VCPY_STK	(34)
#define SYS_VSET_CTX	(35)
#define SYS_VUSE_FPU	(36)

#define SYS_CRE_POR	(37)
#define SYS_ACRE_POR	(38)
#define SYS_DEL_POR	(39)
#define SYS_CAL_POR	(40)
#define SYS_ACP_POR	(41)
#define SYS_RPL_RDV	(42)


extern ER	cre_tsk (ID tskid, T_CTSK *pk_ctsk);
extern ER	del_tsk (ID tskid);
extern ER	sta_tsk (ID tskid, INT stacd);
extern void	ext_tsk (void);
extern void	exd_tsk (void);
extern ER	ter_tsk (ID tskid);
extern ER	chg_pri (ID tskid, PRI tskpri);
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
extern ER vdup_reg (ID src, ID dst, ID rid);
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
extern ER vsys_msc (W cmd, VP argp);
extern ER vsys_inf (W func, W sub, VP argp);
extern ER vcpy_stk(ID src, W esp, W ebp, W ebx, W ecx, W edx, W esi , W edi, ID dst);
extern ER vset_ctx (ID tid, W eip, B *stackp, W stsize);
extern ER vuse_fpu (ID tid);

#endif /*__ITRON_SYSCALL_H__*/
