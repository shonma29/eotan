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
#include "../include/set/slab.h"
#include "../include/set/tree.h"
#include "core.h"
#include "memory.h"
#include "func.h"
#include "arch.h"
#include "eventflag.h"
#include "sync.h"
#include "setting.h"

static slab_t slab;
static tree_t tree;

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
    slab.unit_size = sizeof(T_EVENTFLAG);
    slab.block_size = I386_PAGESIZE;
    slab.min_block = 1;
    slab.max_block = tree_max_block(65536, I386_PAGESIZE, sizeof(T_EVENTFLAG));
    slab.palloc = palloc;
    slab.pfree = pfree;
    slab_create(&slab);

    tree_create(&tree, &slab);

    return (E_OK);
}

ER_ID acre_flg(T_CFLG * pk_flg)
{
    T_EVENTFLAG *p;
    ID flgid;

    if ((pk_flg->flgatr != TA_WSGL) && (pk_flg->flgatr != TA_WMUL)) {
	return (E_PAR);
    }

    for (flgid = MIN_AUTO_ID; flgid <= MAX_AUTO_ID; flgid++) {
	if (!tree_get(&tree, flgid)) {
	    break;
	}
    }

    if (flgid > MAX_AUTO_ID)	return E_ID;

    p = (T_EVENTFLAG*)tree_put(&tree, flgid);
    if (p) {
	return (E_NOMEM);
    }

    list_initialize(&(p->receiver));
    p->flgatr = pk_flg->flgatr;
    p->iflgptn = pk_flg->iflgptn;

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
    T_EVENTFLAG *p = (T_EVENTFLAG*)tree_get(&tree, flgid);
    list_t *q;

    if (!p) {
	return (E_NOEXS);
    }

    while ((q = list_dequeue(&(p->receiver))) != NULL) {
	T_TCB *task = getTaskParent(q);

	task->wait.result = E_DLT;
	release(task);
    }

    tree_remove(&tree, p->node.key);

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
    T_EVENTFLAG *p = (T_EVENTFLAG*)tree_get(&tree, flgid);
    T_TCB *task;
    BOOL result;
    BOOL tsw_flag = FALSE;
    list_t *q, *next;

    if (!p) {
	return (E_NOEXS);
    }

    enter_critical();
    p->iflgptn |= setptn;
    leave_critical();

    for (q = list_next(&(p->receiver));
	    !list_is_edge(&(p->receiver), q); q = next) {
    	next = q->next;
    	task = getTaskParent(q);
	result = FALSE;
	switch ((task->wait.detail.evf.wfmode) & (TWF_ANDW | TWF_ORW)) {
	case TWF_ANDW:
	    result =
		((p->iflgptn & task->wait.detail.evf.waiptn) ==
		 task->wait.detail.evf.waiptn) ? TRUE : FALSE;
	    break;
	case TWF_ORW:
	    result =
		(p->iflgptn & task->
		 wait.detail.evf.waiptn) ? TRUE : FALSE;
	    break;
	}

	if (result == TRUE) {
	    enter_critical();
	    list_remove(q);
	    leave_critical();
	    task->wait.detail.evf.flgptn = p->iflgptn;
	    tsw_flag = TRUE;
	    release(task);
	    if (task->wait.detail.evf.wfmode & TWF_CLR) {
		/* μITORN 3.0 標準ハンドブック p.141 参照 */
		enter_critical();
		p->iflgptn = 0;
		leave_critical();
	    }
	}
    }
    if (tsw_flag) {
	task_switch();
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
    T_EVENTFLAG *p = (T_EVENTFLAG*)tree_get(&tree, flgid);

    if (!p) {
	return (E_NOEXS);
    }

    enter_critical();
    p->iflgptn &= clrptn;
    leave_critical();

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
    T_EVENTFLAG *p;
    BOOL result;

    *flgptn = 0;

    if (waiptn == 0) {
	return (E_PAR);
    }

    p = (T_EVENTFLAG*)tree_get(&tree, flgid);
    if (!p) {
	return (E_NOEXS);
    }

    if ((p->flgatr == TA_WSGL)
	&& !list_is_empty(&(p->receiver))) {
	return (E_OBJ);
    }

    result = FALSE;
    switch (wfmode & (TWF_ANDW | TWF_ORW)) {
    case TWF_ANDW:
	result =
	    ((p->iflgptn & waiptn) == waiptn) ?
	    TRUE : FALSE;
	break;
    case TWF_ORW:
	result = (p->iflgptn & waiptn) ? TRUE : FALSE;
	break;
    }

    if (result == TRUE) {
	*flgptn = p->iflgptn;
	if (wfmode & TWF_CLR) {
	    enter_critical();
	    p->iflgptn = 0;
	    leave_critical();
	}
	return (E_OK);
    }

    enter_critical();
    run_task->wait.detail.evf.waiptn = waiptn;
    run_task->wait.detail.evf.wfmode = wfmode;
    run_task->wait.type = wait_evf;
    run_task->wait.obj_id = flgid;
    list_enqueue(&(p->receiver), &(run_task->wait.waiting));
    leave_critical();
    wait(run_task);
    *flgptn = run_task->wait.detail.evf.flgptn;
    return (run_task->wait.result);
}
