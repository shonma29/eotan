#ifndef _MITRON4_SVC_H_
#define _MITRON4_SVC_H_
/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
#include <mitron4/types.h>
#include <mitron4/packets.h>

/**
 * task
 */
extern ER cre_tsk(ID, T_CTSK *);
extern ER_ID acre_tsk(T_CTSK *);
extern ER del_tsk(ID);
extern ER sta_tsk(ID, VP_INT);
extern void exd_tsk(void);
extern ER ter_tsk(ID);

/**
 * task ipc
 */
extern ER slp_tsk(void);
extern ER wup_tsk(ID);

/**
 * mutex
 */
extern ER cre_mtx(ID, T_CMTX *);
extern ER_ID acre_mtx(T_CMTX *);
extern ER del_mtx(ID);
extern ER loc_mtx(ID);
extern ER ploc_mtx(ID);
extern ER unl_mtx(ID);

/**
 * rendezvous
 */
extern ER cre_por(ID, T_CPOR *);
extern ER del_por(ID);
extern ER_UINT cal_por(ID, RDVPTN, VP, UINT);
extern ER_UINT acp_por(ID, RDVPTN, RDVNO *, VP);
extern ER rpl_rdv(RDVNO, VP, UINT);

/**
 * time
 */
extern ER get_tim(SYSTIM *);
extern ER isig_tim(void);

/**
 * interruption
 */
extern ER def_inh(INHNO, T_DINH *);

#endif
