/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* event.c --- イベントフラグ機能
 *
 *
 */


#include "../include/mpu/io.h"
#include "../include/set/list.h"
#include "core.h"
#include "memory.h"
#include "func.h"
#include "arch.h"
#include "eventflag.h"

static T_EVENTFLAG flag_table[MAX_EVENTFLAG + 1];

static void tflg_func(VP p);
static T_TCB *getTaskParent(const list_t *p);


static T_TCB *getTaskParent(const list_t *p) {
	return (T_TCB*)((ptr_t)p - offsetof(T_TCB, wait.waiting));
}

/*******************************************************************************
 * init_eventflag
 *
 * 引数：
 *
 * 返り値：
 *
 * 機能：
 *
 */
ER init_eventflag(void)
{
    W i;

    for (i = MIN_EVENTFLAG; i <= MAX_EVENTFLAG; i++) {
	flag_table[i].flgatr = TA_UNDEF;
	flag_table[i].id = i;
	list_initialize(&(flag_table[i].receiver));
    }
    return (E_OK);
}

/*******************************************************************************
 * cre_flg --- イベントフラグの生成
 *
 * 引数：
 *
 * 返り値：
 *
 * 機能：
 *
 */
ER cre_flg(ID flgid, T_CFLG * pk_flg)
{
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr != TA_UNDEF) {
	return (E_OBJ);
    }

    if ((pk_flg->flgatr != TA_WSGL) && (pk_flg->flgatr != TA_WMUL)) {
	return (E_PAR);
    }

    flag_table[flgid].flgatr = pk_flg->flgatr;
    flag_table[flgid].iflgptn = pk_flg->iflgptn;
    list_initialize(&(flag_table[flgid].receiver));
    flag_table[flgid].exinf = pk_flg->exinf;
    return (E_OK);
}

ER_ID acre_flg(T_CFLG * pk_flg)
{
    ID flgid;

    if ((pk_flg->flgatr != TA_WSGL) && (pk_flg->flgatr != TA_WMUL)) {
	return (E_PAR);
    }

    for (flgid = MIN_EVENTFLAG; flgid <= MAX_EVENTFLAG; flgid++) {
	if (flag_table[flgid].flgatr == TA_UNDEF) {
	    break;
	}
    }

    if (flgid > MAX_EVENTFLAG)	return E_ID;

    flag_table[flgid].flgatr = pk_flg->flgatr;
    flag_table[flgid].iflgptn = pk_flg->iflgptn;
    list_initialize(&(flag_table[flgid].receiver));
    flag_table[flgid].exinf = pk_flg->exinf;

    return flgid;
}


/*******************************************************************************
 * del_flg --- イベントフラグの削除
 *
 * 引数：
 *
 * 返り値：
 *	エラー番号を返す。
 *	E_OK
 *	E_ID
 *	E_OBJ
 *
 * 機能：
 *
 */
ER del_flg(ID flgid)
{
    T_TCB *p;
    list_t *q;

    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_UNDEF) {
	return (E_OBJ);
    }

    while ((q = list_dequeue(&(flag_table[flgid].receiver))) != NULL) {
	p = getTaskParent(q);
	p->wait.type = wait_none;
	p->wait.result = E_DLT;
	wup_tsk(p->tskid);
    }

    flag_table[flgid].flgatr = TA_UNDEF;
    return (E_OK);
}

/*******************************************************************************
 * set_flg --- イベントフラグに値をセットする
 *
 * 引数：
 *
 * 返り値：
 *	エラー番号を返す。
 *	E_OK
 *	E_ID
 *	E_OBJ
 *
 * 機能：
 *
 */
ER set_flg(ID flgid, UINT setptn)
{
    T_TCB *p;
    BOOL result;
    BOOL tsw_flag = FALSE;
    list_t *q, *next;

    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_UNDEF) {
	return (E_NOEXS);
    }

    dis_int();
    flag_table[flgid].iflgptn |= setptn;
    ena_int();

    for (q = list_next(&(flag_table[flgid].receiver));
	    !list_is_edge(&(flag_table[flgid].receiver), q); q = next) {
    	next = q->next;
    	p = getTaskParent(q);
	result = FALSE;
	switch ((p->wait.detail.evf.wfmode) & (TWF_ANDW | TWF_ORW)) {
	case TWF_ANDW:
	    result =
		((flag_table[flgid].iflgptn & p->wait.detail.evf.waiptn) ==
		 p->wait.detail.evf.waiptn) ? TRUE : FALSE;
	    break;
	case TWF_ORW:
	    result =
		(flag_table[flgid].iflgptn & p->
		 wait.detail.evf.waiptn) ? TRUE : FALSE;
	    break;
	}

	if (result == TRUE) {
	    dis_int();
	    list_remove(q);
	    ena_int();
	    p->wait.detail.evf.flgptn = flag_table[flgid].iflgptn;
	    p->wait.type = wait_none;
	    tsw_flag = TRUE;
	    wup_tsk(p->tskid);
	    if (p->wait.detail.evf.wfmode & TWF_CLR) {
		/* μITORN 3.0 標準ハンドブック p.141 参照 */
		dis_int();
		flag_table[flgid].iflgptn = 0;
		ena_int();
	    }
	}
    }
    if (tsw_flag) {
	task_switch(TRUE);
    }

    return (E_OK);
}

/*******************************************************************************
 * clr_flg --- イベントフラグの値をクリアする
 *
 * 引数：
 *
 * 返り値：
 *	エラー番号を返す。
 *	E_OK
 *	E_ID
 *	E_OBJ
 *
 * 機能：
 *
 */
ER clr_flg(ID flgid, UINT clrptn)
{
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_UNDEF) {
	return (E_NOEXS);
    }

    dis_int();
    flag_table[flgid].iflgptn &= clrptn;
    ena_int();

    return (E_OK);
}


/*******************************************************************************
 * wai_flg --- イベントフラグ待ち
 *
 * 引数：
 *
 * 返り値：
 *
 * 機能：
 *
 */
ER wai_flg(UINT * flgptn, ID flgid, UINT waiptn, UINT wfmode)
{
    BOOL result;
    INT wcnt;
    *flgptn = NULL;

    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_UNDEF) {
	return (E_NOEXS);
    }

    if ((flag_table[flgid].flgatr == TA_WSGL)
	&& !list_is_empty(&(flag_table[flgid].receiver))) {
	return (E_OBJ);
    }
    if (waiptn == 0) {
	return (E_PAR);
    }

    result = FALSE;
    switch (wfmode & (TWF_ANDW | TWF_ORW)) {
    case TWF_ANDW:
	result =
	    ((flag_table[flgid].iflgptn & waiptn) == waiptn) ?
	    TRUE : FALSE;
	break;
    case TWF_ORW:
	result = (flag_table[flgid].iflgptn & waiptn) ? TRUE : FALSE;
	break;
    }

    if (result == TRUE) {
	*flgptn = flag_table[flgid].iflgptn;
	if (wfmode & TWF_CLR) {
	    dis_int();
	    flag_table[flgid].iflgptn = 0;
	    ena_int();
	}
	return (E_OK);
    }

    dis_int();
    run_task->wait.detail.evf.waiptn = waiptn;
    run_task->wait.detail.evf.wfmode = wfmode;
    run_task->wait.type = wait_evf;
    run_task->wait.obj_id = flgid;
    list_enqueue(&(flag_table[flgid].receiver), &(run_task->wait.waiting));
/*  task_switch (TRUE); */
    ena_int();
    can_wup(&wcnt, run_task->tskid);
    slp_tsk();
    *flgptn = run_task->wait.detail.evf.flgptn;
    return (E_OK);
}
