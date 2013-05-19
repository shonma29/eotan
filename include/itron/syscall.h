
#ifndef __ITRON_SYSCALL_H__
#define __ITRON_SYSCALL_H__

/* システムコール関数の定義 */
/* --------------------------------------------------------- */
/* タスク管理機能                                            */
/* --------------------------------------------------------- */

#include "types.h"
#include "struct.h"

extern void	exd_tsk (void);
extern ER	get_tid (ID *p_tskid);

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

#endif /*__ITRON_SYSCALL_H__*/
