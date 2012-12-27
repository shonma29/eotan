/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* message.c --- メッセージバッファ生成
 *
 *
 */


#include "../include/string.h"
#include "../include/mpu/io.h"
#include "../include/set/list.h"
#include "core.h"
#include "memory.h"
#include "func.h"
#include "arch.h"
#include "message.h"
#include "sync.h"

static T_MSGHEAD message_table[MAX_MSGBUF + 1];
static T_MSG msgbuf[MAX_MSGENTRY];
static list_t unused_msg;

static T_MSG *getMessageParent(const list_t *p);
static T_TCB *getTaskParent(const list_t *p);


static T_MSG *getMessageParent(const list_t *p) {
	return (T_MSG*)((ptr_t)p - offsetof(T_MSG, message));
}

static T_TCB *getTaskParent(const list_t *p) {
	return (T_TCB*)((ptr_t)p - offsetof(T_TCB, wait.waiting));
}

/****************************************************************************
 * alloc_msg
 */
static T_MSG *alloc_msg(void)
{
    T_MSG *p;
    list_t *q;

    enter_serialize();
    q = list_pop(&unused_msg);
    p = q? getMessageParent(q):NULL;
    leave_serialize();

    return (p);
}

/****************************************************************************
 * dealloc_msg
 */
static void dealloc_msg(T_MSG * p)
{
    if (p == NULL)
	return;

    enter_serialize();
    list_push(&unused_msg, &(p->message));
    leave_serialize();
}

/***************************************************************************
 * add_msg_list --- メッセージリストにメッセージを追加する。
 *                  メッセージリストは FIFO (キュー) 
 *
 */
static void add_msg_list(T_MSGHEAD * list, T_MSG * newmsg)
{
    enter_serialize();
    list_enqueue(&(list->message), &(newmsg->message));
    leave_serialize();
}

/***************************************************************************
 * get_msg --- メッセージリストからメッセージを取り出す
 *
 */
static T_MSG *get_msg(T_MSGHEAD * list)
{
    T_MSG *p;
    list_t *q;

    enter_serialize();
    q = list_dequeue(&(list->message));
    p = q? getMessageParent(q):NULL;
    leave_serialize();

    return (p);
}

/****************************************************************************
 * init_msgbuf --- メッセージバッファ機能の初期化
 *
 * 引数：
 *	なし
 *
 * 返り値：
 *	エラー番号
 */
ER queue_initialize(void)
{
    INT i;

    list_initialize(&unused_msg);

    for (i = 0; i < MAX_MSGENTRY; i++) {
	list_enqueue(&unused_msg, &(msgbuf[i].message));
    }

    for (i = 0; i <= MAX_MSGBUF; i++) {
	message_table[i].mbfatr = TA_UNDEF;
	list_initialize(&(message_table[i].sender));
	list_initialize(&(message_table[i].receiver));
    }
    return (E_OK);
}


/****************************************************************************
 * queue_create -- メッセージバッファ生成
 *
 * 引数：
 *	id	メッセージバッファ ID
 *	pk_cmbf	メッセージバッファ生成情報
 *		mbfatr	メッセージ属性 (TA_FIFO 固定)
 *		bufsz	バッファサイズ
 *		maxmsz	メッセージの最大長
 *
 * 返り値：
 *	ER	エラー番号
 *		E_OK	正常終了
 *		E_ID	メッセージ ID 不正
 *		E_OBJ	同一ID のメッセージがすでに存在する
 *		E_PAR	パラメータ不正
 *
 * 機能：
 *	メッセージバッファを生成する。
 */
ER queue_create(ID id, T_CMBF * pk_cmbf)
{
    if ((id < MIN_MSGBUF) || (id > MAX_MSGBUF)) {
	return (E_ID);
    }
    if ((pk_cmbf->mbfatr != TA_TFIFO) || (pk_cmbf->bufsz < 0)
	|| (pk_cmbf->maxmsz < 0)) {
	return (E_PAR);
    }
    if (message_table[id].mbfatr != TA_UNDEF) {
	return (E_OBJ);
    }

    enter_serialize();
    message_table[id].total_size = pk_cmbf->bufsz;
    list_initialize(&(message_table[id].message));
    message_table[id].msgsz = pk_cmbf->maxmsz;
    message_table[id].bufsz = pk_cmbf->bufsz;
    message_table[id].mbfatr = pk_cmbf->mbfatr;
    leave_serialize();

    return (E_OK);
}

ER_ID queue_create_auto(T_CMBF * pk_cmbf)
{
    ID id;

    if ((pk_cmbf->mbfatr != TA_TFIFO) || (pk_cmbf->bufsz < 0)
	|| (pk_cmbf->maxmsz < 0)) {
	return E_PAR;
    }

    for (id = MIN_MSGBUF; id <= MAX_MSGBUF; id++) {
	if (message_table[id].mbfatr == TA_UNDEF) {
	    break;
	}
    }

    if (id > MAX_MSGBUF) {
	return E_NOID;
    }

    enter_serialize();
    message_table[id].total_size = pk_cmbf->bufsz;
    list_initialize(&(message_table[id].message));
    message_table[id].msgsz = pk_cmbf->maxmsz;
    message_table[id].bufsz = pk_cmbf->bufsz;
    message_table[id].mbfatr = pk_cmbf->mbfatr;
    leave_serialize();

    return id;
}

/**************************************************************************************
 * queue_destroy --- メッセージバッファを削除する
 *
 * 引数：
 *	id	メッセージバッファ ID
 *
 * 返り値：
 *	ER	エラー番号
 *		E_OK	正常終了
 *		E_ID	メッセージ ID 不正
 *		E_NOEXS メッセージバッファが存在しない
 *
 * 機能：
 *	引数 ID で指定したメッセージバッファを削除する。
 *
 */
ER queue_destroy(ID id)
{
    T_TCB *p;
    T_MSG *msgp;
    list_t *q;

    if ((id < MIN_MSGBUF) || (id > MAX_MSGBUF)) {
	return (E_ID);
    }
    if (message_table[id].mbfatr == TA_UNDEF) {
	return (E_NOEXS);
    }

    while ((q = list_dequeue(&(message_table[id].message))) != NULL) {
        msgp = getMessageParent(q);
	kfree(msgp->buf, msgp->size);
	dealloc_msg(msgp);
    }

    /* 受信待ちタスクに対して E_DLT を返す */
    while ((q = list_dequeue(&(message_table[id].receiver))) != NULL) {
	p = getTaskParent(q);
	p->wait.result = E_DLT;
	release(p);
    }

    /* 送信待ちタスクに対して E_DLT を返す */
    while ((q = list_dequeue(&(message_table[id].sender))) != NULL) {
	p = getTaskParent(q);
	p->wait.result = E_DLT;
	release(p);
    }

    enter_serialize();
    message_table[id].mbfatr = TA_UNDEF;
    leave_serialize();

    return (E_OK);
}

/*********************************************************************************
 * message_send --- メッセージバッファへ送信
 *
 * 引数：
 *	id	メッセージバッファ ID
 *	size	送信メッセージのサイズ
 *	msg	送信メッセージの先頭アドレス
 *
 * 返り値：
 *	ER	エラー番号
 *
 * 機能：
 *	
 */
ER message_send(ID id, INT size, VP msg)
{
    T_TCB *p;
    T_MSG *newmsg;
    VP buf;
    BOOL tsksw = FALSE;
    list_t *q;

    if ((id < MIN_MSGBUF) || (id > MAX_MSGBUF)) {
	return (E_ID);
    }
    if (message_table[id].mbfatr == TA_UNDEF) {
	return (E_NOEXS);
    }
    if (size > message_table[id].msgsz) {
	return (E_PAR);
    }

    if (message_table[id].bufsz == 0) {
	enter_serialize();
	q = list_dequeue(&(message_table[id].receiver));

	if (!q) {
	    /* 受信を待っているタスクが無ければ sleep */
	    list_enqueue(&(message_table[id].sender),
		    &(run_task->wait.waiting));
	    run_task->wait.detail.msg.size = size;
	    run_task->wait.detail.msg.msg = msg;
	    run_task->wait.type = wait_msg;
	    enter_critical();
	    leave_serialize();

	    wait(run_task);
	    return (run_task->wait.result);

	} else {
	    /* もし、受信待ちタスクがあれば、message を転送して wakeup する */
	    leave_serialize();

	    p = getTaskParent(q);
	    vput_reg(p->tskid, p->wait.detail.msg.msg, size, msg);
	    p->wait.detail.msg.size = size;
	    tsksw = TRUE;
	    release(p);
	}
    } else {
      retry:
	newmsg = alloc_msg();
	if ((newmsg == NULL) || (message_table[id].total_size < size)) {
	    /* メッセージバッファがアロケートできなかった。 */
	    /* 送信待ちリストに入れ、sleep する。           */
	    enter_serialize();
	    list_enqueue(&(message_table[id].sender),
		    &(run_task->wait.waiting));
	    run_task->wait.type = wait_msg;
	    enter_critical();
	    leave_serialize();

	    wait(run_task);
	    dealloc_msg(newmsg);
	    if (run_task->wait.result) {
		return (run_task->wait.result);
	    }
	    goto retry;
	}

	buf = kalloc(size);
	if (buf == NULL) {
	    dealloc_msg(newmsg);
	    return (E_OBJ);
	}

	newmsg->buf = buf;
	newmsg->size = size;
	add_msg_list(&message_table[id], newmsg);
	message_table[id].total_size -= size;
	memcpy(buf, msg, size);

	/* もし、受信待ちタスクがあれば、それを wakeup する */
	if (!list_is_empty(&(message_table[id].receiver))) {
	    enter_serialize();
	    q = list_dequeue(&(message_table[id].receiver));
	    leave_serialize();

	    p = getTaskParent(q);
	    tsksw = TRUE;
	    release(p);
	}
    }

    return (E_OK);
}

/*********************************************************************************
 * message_send_nowait --- メッセージバッファへ送信(ポーリング)
 *
 * 引数：
 *	id	メッセージバッファ ID
 *	size	送信メッセージのサイズ
 *	msg	送信メッセージの先頭アドレス
 *
 * 返り値：
 *	ER	エラー番号
 *
 * 機能：
 *	
 */
ER message_send_nowait(ID id, INT size, VP msg)
{
    T_TCB *p;
    T_MSG *newmsg;
    VP buf;
    BOOL tsksw = FALSE;
    list_t *q;

    if ((id < MIN_MSGBUF) || (id > MAX_MSGBUF)) {
	return (E_ID);
    }
    if (message_table[id].mbfatr == TA_UNDEF) {
	return (E_NOEXS);
    }
    if (size > message_table[id].msgsz) {
	return (E_PAR);
    }

    if (message_table[id].bufsz == 0) {
	enter_serialize();
	q = list_dequeue(&(message_table[id].receiver));
	leave_serialize();

	if (!q) {
	    /* 受信を待っているタスクが無ければ E_TMOUT を返す */
	    return (E_TMOUT);
	} else {
	    /* もし、受信待ちタスクがあれば、message を転送して wakeup する */
	    p = getTaskParent(q);
	    vput_reg(p->tskid, p->wait.detail.msg.msg, size, msg);
	    p->wait.detail.msg.size = size;
	    tsksw = TRUE;
	    release(p);
	}
    } else {
	newmsg = alloc_msg();
	if ((newmsg == NULL) || (message_table[id].total_size < size)) {
	    /* メッセージバッファがアロケートできなかった。 */
	    /* E_TMOUT のエラーを返す                       */
	    return (E_TMOUT);
	}

	buf = kalloc(size);
	if (buf == NULL) {
	    dealloc_msg(newmsg);
	    return (E_TMOUT);
	}

	newmsg->buf = buf;
	newmsg->size = size;
	add_msg_list(&message_table[id], newmsg);
	message_table[id].total_size -= size;
	memcpy(buf, msg, size);

	/* もし、受信待ちタスクがあれば、それを wakeup する */
	if (!list_is_empty(&(message_table[id].receiver))) {
	    enter_serialize();
	    q = list_dequeue(&(message_table[id].receiver));
	    leave_serialize();

	    p = getTaskParent(q);
	    tsksw = TRUE;
	    release(p);
	}
    }

    return (E_OK);
}

/*********************************************************************************
 * message_receive --- メッセージバッファから受信する
 *
 * 引数：
 *	msg	受信したメッセージを入れる領域
 *	id	メッセージバッファ ID
 *	size	受信したメッセージのサイズ(返り値)
 *
 * 返り値：
 *	ER	エラー番号
 *
 * 機能：
 *
 */
ER message_receive(VP msg, INT * size, ID id)
{
    T_TCB *p;
    T_MSG *newmsgp;
    list_t *q;

    if ((id < MIN_MSGBUF) || (id > MAX_MSGBUF)) {
	return (E_ID);
    }
    if (message_table[id].mbfatr == TA_UNDEF) {
	return (E_NOEXS);
    }

    if (message_table[id].bufsz == 0) {
	enter_serialize();
	q = list_dequeue(&(message_table[id].sender));

	if (!q) {
	    list_enqueue(&(message_table[id].receiver),
		    &(run_task->wait.waiting));
	    run_task->wait.detail.msg.size = 0;
	    run_task->wait.detail.msg.msg = msg;
	    run_task->wait.type = wait_msg;
	    enter_critical();
	    leave_serialize();

	    wait(run_task);
	    *size = run_task->wait.detail.msg.size;
	    return (run_task->wait.result);

	} else {
	    leave_serialize();

	    p = getTaskParent(q);
	    *size = p->wait.detail.msg.size;
	    vget_reg(p->tskid, p->wait.detail.msg.msg,
		    p->wait.detail.msg.size, msg);
	    release(p);
	}
    } else {
	/* メッセージが存在しない --> sleep する。 */
      retry:
	if (list_is_empty(&(message_table[id].message))) {
	    enter_serialize();
	    list_enqueue(&(message_table[id].receiver),
		    &(run_task->wait.waiting));
	    run_task->wait.type = wait_msg;
	    enter_critical();
	    leave_serialize();

	    wait(run_task);

	    if (run_task->wait.result) {
		return (run_task->wait.result);
	    }
	    goto retry;
	}

	/* メッセージリストから1つエントリを取り出す */
	newmsgp = get_msg(&message_table[id]);
	/* メッセージの内容をコピーする */
	memcpy(msg, (VP)newmsgp->buf, newmsgp->size);
	*size = (INT) newmsgp->size;
	kfree(newmsgp->buf, *size);
	dealloc_msg(newmsgp);
	message_table[id].total_size += *size;

	/* 送信待ちのタスクがあれば wakeup する */
	if (!list_is_empty(&(message_table[id].sender))) {
	    enter_serialize();
	    q = list_dequeue(&(message_table[id].sender));
	    leave_serialize();

	    p = getTaskParent(q);
	    release(p);
	}
    }

    return (E_OK);
}
