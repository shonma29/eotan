/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* event.c --- ���٥�ȥե饰��ǽ
 *
 *
 */


#include "itron.h"
#include "config.h"
#include "memory.h"
#include "errno.h"
#include "func.h"
#include "ibmpc.h"
#include "eventflag.h"

static T_EVENTFLAG flag_table[MAX_EVENTFLAG + 1];

static void tflg_func(VP p);
static void del_evt_wait(ID fid, ID tid);

/*******************************************************************************
 * init_eventflag
 *
 * ������
 *
 * �֤��͡�
 *
 * ��ǽ��
 *
 */
ER init_eventflag(void)
{
    W i;

    for (i = MIN_EVENTFLAG; i <= MAX_EVENTFLAG; i++) {
	flag_table[i].flgatr = TA_FREE;
	flag_table[i].id = i;
	flag_table[i].wait_task = NULL;
	flag_table[i].wait_tail = NULL;
    }
    return (E_OK);
}

/*******************************************************************************
 * cre_flg --- ���٥�ȥե饰������
 *
 * ������
 *
 * �֤��͡�
 *
 * ��ǽ��
 *
 */
ER cre_flg(ID flgid, T_CFLG * pk_flg)
{
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr != TA_FREE) {
	return (E_OBJ);
    }

    if ((pk_flg->flgatr != TA_WSGL) && (pk_flg->flgatr != TA_WMUL)) {
	return (E_PAR);
    }

    flag_table[flgid].flgatr = pk_flg->flgatr;
    flag_table[flgid].iflgptn = pk_flg->iflgptn;
    flag_table[flgid].wait_task = NULL;
    flag_table[flgid].wait_tail = NULL;
    flag_table[flgid].exinf = pk_flg->exinf;
    return (E_OK);
}


/*******************************************************************************
 * del_flg --- ���٥�ȥե饰�κ��
 *
 * ������
 *
 * �֤��͡�
 *	���顼�ֹ���֤���
 *	E_OK
 *	E_ID
 *	E_OBJ
 *
 * ��ǽ��
 *
 */
ER del_flg(ID flgid)
{
    T_TCB *p;

    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_OBJ);
    }

    for (p = flag_table[flgid].wait_task; p != NULL; p = p->event_next) {
	p->tskwait.event_wait = 0;
	p->slp_err = E_TMOUT;
	wup_tsk(p->tskid);
    }

    flag_table[flgid].flgatr = TA_FREE;
    flag_table[flgid].wait_task = NULL;
    flag_table[flgid].wait_tail = NULL;
    return (E_OK);
}

/*******************************************************************************
 * set_flg --- ���٥�ȥե饰���ͤ򥻥åȤ���
 *
 * ������
 *
 * �֤��͡�
 *	���顼�ֹ���֤���
 *	E_OK
 *	E_ID
 *	E_OBJ
 *
 * ��ǽ��
 *
 */
ER set_flg(ID flgid, UINT setptn)
{
    T_TCB *p, *q;
    BOOL result;
    BOOL tsw_flag = FALSE;

    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_NOEXS);
    }

/*  printk ("set_flag: count = %d\n", list_counter (flag_table[flgid].wait_task)); */
#ifdef notdef
    printk("set_flg id %d ptn %x\n", flgid, setptn);
#endif
    dis_int();
    flag_table[flgid].iflgptn |= setptn;
    ena_int();

    q = NULL;
    for (p = flag_table[flgid].wait_task; p != NULL; p = p->event_next) {
	result = FALSE;
	switch ((p->wfmode) & (TWF_ANDW | TWF_ORW)) {
	case TWF_ANDW:
	    result =
		((flag_table[flgid].iflgptn & p->flag_pattern) ==
		 p->flag_pattern) ? TRUE : FALSE;
	    break;
	case TWF_ORW:
	    result =
		(flag_table[flgid].iflgptn & p->
		 flag_pattern) ? TRUE : FALSE;
	    break;
	}

	if (result == TRUE) {
	    dis_int();
	    if (p->event_next == NULL) {
		flag_table[flgid].wait_tail = q;
	    }
	    if (q == NULL) {
		flag_table[flgid].wait_task = p->event_next;
	    } else {
		q->event_next = p->event_next;
	    }
#ifdef notdef
	    printk("eventflag: wup tsk %d\n", p->tskid);
#endif
	    ena_int();
	    p->rflgptn = flag_table[flgid].iflgptn;
	    p->tskwait.event_wait = 0;
	    tsw_flag = TRUE;
	    wup_tsk(p->tskid);
	    if (p->wfmode & TWF_CLR) {
		/* ��ITORN 3.0 ɸ��ϥ�ɥ֥å� p.141 ���� */
		dis_int();
		flag_table[flgid].iflgptn = 0;
		ena_int();
	    }
	} else {
	    q = p;
	}
    }
    if (tsw_flag) {
#ifdef notdef
	printk("eventflag: task_switch\n");
#endif
	task_switch(TRUE);
    }

    return (E_OK);
}

/*******************************************************************************
 * clr_flg --- ���٥�ȥե饰���ͤ򥯥ꥢ����
 *
 * ������
 *
 * �֤��͡�
 *	���顼�ֹ���֤���
 *	E_OK
 *	E_ID
 *	E_OBJ
 *
 * ��ǽ��
 *
 */
ER clr_flg(ID flgid, UINT clrptn)
{
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_NOEXS);
    }

    dis_int();
    flag_table[flgid].iflgptn &= clrptn;
    ena_int();

    return (E_OK);
}


/*******************************************************************************
 * wai_flg --- ���٥�ȥե饰�Ԥ�
 *
 * ������
 *
 * �֤��͡�
 *
 * ��ǽ��
 *
 */
ER wai_flg(UINT * flgptn, ID flgid, UINT waiptn, UINT wfmode)
{
    BOOL result;
    INT wcnt;

#ifdef notdef
    printk("wai_flg id %d ptn %x\n", flgid, waiptn);
#endif
    *flgptn = NULL;
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_NOEXS);
    }

    if ((flag_table[flgid].flgatr == TA_WSGL)
	&& (flag_table[flgid].wait_task)) {
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
#if 0
    /* for debug */
    printk("wai_flg: before = %d  ", list_counter(flag_table[flgid].wait_task));	/* debug */
#endif

    dis_int();
    run_task->flag_pattern = waiptn;
    run_task->wfmode = wfmode;
    run_task->tskwait.event_wait = 1;
    run_task->event_id = flgid;
    run_task->event_next = NULL;
    if (flag_table[flgid].wait_tail != NULL) {
	flag_table[flgid].wait_tail->event_next = run_task;
    }
    flag_table[flgid].wait_tail = run_task;
    if (flag_table[flgid].wait_task == NULL) {
	flag_table[flgid].wait_task = run_task;
    }

#if 0
/* for debug */
    printk("wai_flg: after = %d\n", list_counter(flag_table[flgid].wait_task));	/* debug */
#endif

/*  task_switch (TRUE); */
    ena_int();
    can_wup(&wcnt, run_task->tskid);
    slp_tsk();
    *flgptn = run_task->rflgptn;
    return (E_OK);
}


/*******************************************************************************
 * pol_flg --- ���٥�ȥե饰�Ԥ�(�ݡ�����դ�)
 *
 * ������
 *
 * �֤��͡�
 *
 * ��ǽ��
 *
 */
ER pol_flg(UINT * flgptn, ID flgid, UINT waiptn, UINT wfmode)
{
    *flgptn = NULL;
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_NOEXS);
    }

    if ((flag_table[flgid].flgatr == TA_WSGL)
	&& (flag_table[flgid].wait_task)) {
	return (E_OBJ);
    }
    if (waiptn == 0) {
	return (E_PAR);
    }

    switch ((wfmode) & (TWF_ANDW | TWF_ORW)) {
    case TWF_ANDW:
	if (!((flag_table[flgid].iflgptn & waiptn) == waiptn)) {
	    return (E_TMOUT);
	}
	/* DO NOT REACHED */

    case TWF_ORW:
	if (!(flag_table[flgid].iflgptn & waiptn)) {
	    return (E_TMOUT);
	}
	/* DO NOT REACHED */

    default:
	return (E_TMOUT);
    }

    *flgptn = flag_table[flgid].iflgptn;
    if (wfmode & TWF_CLR) {
	flag_table[flgid].iflgptn = 0;
    }

    return (E_OK);
}


/*******************************************************************************
 * twai_flg  --- ���٥�ȥե饰�Ԥ�(�����ॢ�����դ�)
 *
 * ������
 *
 * �֤��͡�
 *
 * ��ǽ��
 *
 */
ER twai_flg(UINT * flgptn, ID flgid, UINT waiptn, UINT wfmode, TMO tmout)
{
    BOOL result;
    INT wcnt;

    *flgptn = NULL;
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_NOEXS);
    }

    if ((flag_table[flgid].flgatr == TA_WSGL)
	&& (flag_table[flgid].wait_task)) {
	return (E_OBJ);
    }
    if (waiptn == 0) {
	return (E_PAR);
    }
    if (tmout < -1) {
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
    } else if (tmout == 0) {
	*flgptn = flag_table[flgid].iflgptn;
	if (wfmode & TWF_CLR) {
	    dis_int();
	    flag_table[flgid].iflgptn = 0;
	    ena_int();
	}
	return (E_TMOUT);
    }

    dis_int();
    run_task->slp_err = E_OK;
    run_task->tskwait.event_wait = 1;
    run_task->event_id = flgid;
    run_task->event_next = NULL;
    if (flag_table[flgid].wait_tail != NULL) {
	flag_table[flgid].wait_tail->event_next = run_task;
    }
    flag_table[flgid].wait_tail = run_task;
    if (flag_table[flgid].wait_task == NULL) {
	flag_table[flgid].wait_task = run_task;
    }
    ena_int();
    if (tmout > 0) {
	set_timer(tmout, (void (*)(VP)) tflg_func, run_task);
    }
    can_wup(&wcnt, run_task->tskid);
    slp_tsk();
    unset_timer((void (*)(VP)) tflg_func, run_task);
    *flgptn = run_task->rflgptn;
    if (run_task->slp_err) {
	if (run_task->slp_err == E_TMOUT) {
	    del_evt_wait(flgid, run_task->tskid);
	}
	return (run_task->slp_err);
    } else {
	return (E_OK);
    }
}

/*******************************************************************************
 * ref_flg --- ���٥�ȥե饰���ֻ���
 *
 * ������
 *
 * �֤��͡�
 *
 * ��ǽ��
 *
 */
ER ref_flg(T_RFLG * pk_rflg, ID flgid)
{
    if (flgid < MIN_EVENTFLAG || flgid > MAX_EVENTFLAG) {
	return (E_ID);
    }

    if (flag_table[flgid].flgatr == TA_FREE) {
	return (E_NOEXS);
    }

    pk_rflg->exinf = flag_table[flgid].exinf;
    pk_rflg->wtsk = flag_table[flgid].wait_task ? TRUE : FALSE;
    pk_rflg->flgptn = flag_table[flgid].iflgptn;
    return (E_OK);
}

/*********************************************************************************
 * tflg_func --- �����ॢ���Ȼ��֤��᤮�����˼¹Ԥ���ؿ�
 *
 * ������
 *	�����ॢ���Ȥ����������Σԣã��ΰ�ؤΥݥ���
 *
 * �֤��͡�
 *	�ʤ�
 *
 * ��ǽ��
 *	�ܴؿ��ϡ�twai_flg �ˤ�äƥ��٥�ȥե饰�Ԥ����Ƥ�����������
 *	�����ॢ���Ȥ������˸ƤӤ�����롣
 */
static void tflg_func(VP p)
{
    T_TCB *taskp;
#ifdef notdef
    T_EVENTFLAG *eventp;
#endif

    taskp = (T_TCB *) p;
    dis_int();
    taskp->slp_err = E_TMOUT;
    taskp->tskwait.event_wait = 0;

#ifdef notdef
    eventp = &flag_table[taskp->event_id];
    eventp->wait_task = del_tcb_list(eventp->wait_task, taskp);
#endif
    taskp->rflgptn = NULL;
    ena_int();
    wup_tsk(taskp->tskid);
}

/* del_task_evt -- ���٥�ȥե饰���饿��������
 *
 *
 */

static void del_evt_wait(ID fid, ID tid)
{
    T_TCB *p, *q;

    if (flag_table[fid].flgatr == TA_FREE)
	return;
    dis_int();
    q = NULL;
    for (p = flag_table[fid].wait_task; p != NULL; p = p->event_next) {
	if (p->tskid == tid) {
	    if (p->event_next == NULL) {
		flag_table[fid].wait_tail = q;
	    }
	    if (q == NULL) {
		flag_table[fid].wait_task = p->event_next;
	    } else {
		q->event_next = p->event_next;
	    }
	} else {
	    q = p;
	}
    }
    ena_int();
}

void del_task_evt(ID tid)
{
    ID fid;

    for (fid = MIN_EVENTFLAG; fid <= MAX_EVENTFLAG; ++fid) {
	del_evt_wait(fid, tid);
    }
}
