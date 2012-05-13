/*

  B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
  
  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991
  
  (C) B-Free Project.

  (C) 2001-2003, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/wconsole/main.c,v 1.6 2000/04/03 14:30:42 naniwa Exp $ */
static char rcsid[] =
    "$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/wconsole/main.c,v 1.6 2000/04/03 14:30:42 naniwa Exp $";

/*
 * $Log: main.c,v $
 * Revision 1.6  2000/04/03 14:30:42  naniwa
 * to call timer handler in task
 *
 * Revision 1.5  2000/02/27 15:29:24  naniwa
 * to work as multi task OS
 *
 * Revision 1.4  2000/02/20 09:34:15  naniwa
 * to correspond to writing from device
 *
 * Revision 1.3  2000/02/19 03:00:11  naniwa
 * to support multi window environment
 *
 * Revision 1.2  2000/01/29 16:16:14  naniwa
 * to work with POSIX fcntl
 *
 * Revision 1.1  1999/05/15 09:56:04  naniwa
 * first version
 *
 */

/**************************************************************************
 Discription

**********************************************************************/

#include "wconsole_int.h"
#include "../../kernel/servers/port-manager.h"
#include "../../kernel/lib/frtm/frtm.h"

#define USE_MALLOC
#define START_FROM_INIT

/*********************************************************************
 *	 局所変数群の宣言
 *
 */
static void main_loop();
static void doit(DDEV_REQ * packet);

static REQ_LIST req_list[MAX_REQ_LIST], *free_req_list = NULL, *cmsg =
    NULL;

static int key_mode;

/*********************************************************************
 *	 大域変数群の宣言
 *
 */
ID recvport;
W initialized = 0;
ID recv, console, keyboard;

extern int focus, max_win;
extern struct window w[];


/*
 * wconsole デバイスドライバの main 関数
 *
 * この関数は、デバイスドライバ立ち上げ時に一回だけ実行する。
 *
 */
start()
{
    extern char version[];

    /* 
     * 要求受信用のポートの作成
     */
    init_wconsole();

    /*
     * 立ち上げメッセージ
     */
    dbg_printf("wconsole manager start\n", 0);

    main_loop();
}

static void main_loop()
{
    DDEV_REQ req;		/* 受信する要求パケット */
    DDEV_RES res;
    ER err;
    ER errno;
    INT rsize;

    /*
     * 要求受信 - 処理のループ
     */
    for (;;) {
	rsize = sizeof(req);
	errno = rcv_mbf(&req, &rsize, recvport);
	switch (errno) {
	case E_OK:
	    /* 正常ケース */
	    doit(&req);
	    break;
	case E_TMOUT:
	    break;
	default:
	    /* Unknown error */
#ifdef DEBUG
	    dbg_printf("WCONSOLE: get_req() Unknown error(error = %d)\n",
		       err);
#endif
	    break;
	}

	if ((cmsg == NULL) && (w[focus].list != NULL)) {
	    /* 次のメッセージの獲得 */
	    cmsg = w[focus].list;
	    w[focus].list = cmsg->next;
	    if (w[focus].list == cmsg)
		w[focus].list = NULL;
	    cmsg->prev->next = cmsg->next;
	    cmsg->next->prev = cmsg->prev;

	    /* keyboard への read の発行 */
	    req.header.mbfid = recvport;
	    req.header.msgtyp = DEV_RLY;
	    req.body.rea_req.dd = focus;
	    req.body.rea_req.start = cmsg->msg.body.prd_req.start;
	    req.body.rea_req.size = cmsg->msg.body.prd_req.length;
	    errno = snd_mbf(keyboard, sizeof(req), &req);

	    /* 送信に失敗した場合 */
	    if (errno != E_OK) {
		dbg_printf("WCONSOLE: cant send message to keyboard\n");
	    }
	}
    }

    /* ここの行には、来ない */
}


/*
 * 初期化
 *
 * o 要求受けつけ用のメッセージバッファ ID をポートマネージャに登録
 */

static void init_req_list()
{
    int i;

    for (i = 0; i < MAX_REQ_LIST - 1; ++i) {
	req_list[i].next = &req_list[i + 1];
    }
    req_list[MAX_REQ_LIST - 1].next = NULL;
    free_req_list = &req_list[0];
}

W init_wconsole(void)
{
    int i;
    ER error;
    W x, y;
    struct posix_request req;
    struct posix_response res;
    ID posix_port;
    INT rsize;
#ifndef START_FROM_INIT
    ID myself;
#endif

    /*
     * 要求受けつけ用のポートを初期化する。
     */
    recvport = get_port(0, sizeof(DDEV_REQ));

    if (recvport <= 0) {
	dbg_printf("WCONSOLE: cannot make receive port.\n");
	slp_tsk();
	/* メッセージバッファ生成に失敗 */
    }

    error = regist_port(WCONSOLE_DRIVER, recvport);
    if (error != E_OK) {
	dbg_printf("WCONSOLE: cannot regist port (error = %d)\n", error);
    }
    initialized = 1;
    dbg_printf("WCONSOLE: regist port %d\n", recvport);

    recv = get_port(0, sizeof(DDEV_RES));

    if (recv <= 0) {
	dbg_printf("WCONSOLE: cannot make receive port for console.\n");
	slp_tsk();
    }

    if (find_port(CONSOLE_DRIVER, &console) != E_PORT_OK) {
	dbg_printf("WCONSOLE: cannot find port for %s.\n", CONSOLE_DRIVER);
	slp_tsk();
    }
    if (find_port(KEYBOARD_DRIVER, &keyboard) != E_PORT_OK) {
	dbg_printf("WCONSOLE: cannot find port for %s.\n",
		   KEYBOARD_DRIVER);
	slp_tsk();
    }

    /* enter POSIX */
    dbg_printf("WCONSOLE: enter POSIX\n");
    if (find_port("manager.posix", &posix_port) != E_PORT_OK) {
	dbg_printf("WCONSOLE: cannot find port for %s.\n",
		   "manager.posix");
	slp_tsk();
    }
#ifndef START_FROM_INIT
    get_tid(&myself);
    req.receive_port = recvport;
    req.msg_length = sizeof(res);
    req.operation = PSC_MISC;
    req.procid = 1;
    req.caller = myself;
    req.param.par_misc.cmd = M_ENTER_POSIX;
    snd_mbf(posix_port, sizeof(req), &req);
    rsize = sizeof(res);
    rcv_mbf(&res, &rsize, recvport);
#endif

    if (vget_csl(&x, &y) == E_OK) {
	set_curpos(x, y, 0);
    } else {
	clear_wconsole(0);
    }

    /* request list の初期化 */
    init_req_list();
#ifdef USE_MALLOC
    init_malloc(VADDR_HEAP);
#endif

    key_mode = WAITMODE | ENAEOFMODE;
}


/************************************************************************
 *
 *
 */
static void doit(DDEV_REQ * packet)
{
    switch (packet->header.msgtyp) {
    case DEV_OPN:
	/* デバイスのオープン */
	if (!initialized) {
	    init_wconsole();
	}
	open_wconsole(packet->header.mbfid, &(packet->body.opn_req));
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_wconsole(packet->header.mbfid, &(packet->body.cls_req));
	break;

    case DEV_REA:
	read_wconsole(packet->header.mbfid, &(packet->body.rea_req));
	break;

    case DEV_PRD:
	posix_read_wconsole(packet->header.mbfid, packet->header.tskid,
			    packet);
	break;

    case DEV_WRI:
	write_wconsole(packet->header.mbfid, &(packet->body.wri_req));
	break;

    case DEV_CTL:
	control_wconsole(packet->header.mbfid, packet->header.tskid,
			 &(packet->body.ctl_req));
	break;

    case DEV_RLY:
	relay_wconsole(packet->header.mbfid, &(packet->body.rly_req));
	break;
    }
}

/************************************************************************
 * open_wconsole --- wconsole のオープン
 *
 * 引数：	caller	メッセージの送り手
 *		packet	オープンパケット
 *
 * 返値：	常に E_OK を返す。
 *
 * 処理：	E_OK をメッセージの送り手に返す。
 *
 */
W open_wconsole(ID caller, DDEV_OPN_REQ * packet)
{
    DDEV_RES res;

    res.body.opn_res.dd = packet->dd;
    res.body.opn_res.size = 0;
    res.body.opn_res.errcd = E_OK;
    res.body.opn_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/************************************************************************
 * close_wconsole --- ドライバのクローズ
 *
 * 引数：	dd	wconsole ドライバ番号
 *		o_mode	オープンモード
 *		error	エラー番号
 *
 * 返値：	常に E_OK を返す。
 *
 * 処理：	コンソールはクローズの処理ではなにもしない。
 *
 */
W close_wconsole(ID caller, DDEV_CLS_REQ * packet)
{
    DDEV_RES res;

    res.body.cls_res.dd = packet->dd;
    res.body.cls_res.errcd = E_OK;
    res.body.cls_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/*************************************************************************
 * read_wconsole --- 
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	E_NOSPT を返す。
 *
 * 処理：	メッセージの送り手に E_NOSPT を返す。
 *		コンソールドライバは表示するだけなので入力はサポートして
 *		いない。
 *
 */
W read_wconsole(ID caller, DDEV_REA_REQ * packet)
{
    DDEV_RES res;

    res.body.rea_res.dd = packet->dd;
    res.body.rea_res.errcd = E_NOSPT;
    res.body.rea_res.errinfo = E_NOSPT;
    snd_mbf(caller, sizeof(res), &res);
    return (E_NOSPT);
}

/*************************************************************************
 * posix_read_wconsole --- 
 *
 * 引数：	caller
 *	tskid,	packet
 *
 * 返値：
 *
 * 処理： keyboard への入力を取り次ぐ。
 *	
 *	
 *
 */
W posix_read_wconsole(ID caller, ID tskid, DDEV_REQ * packet)
{
    struct posix_response pres;
    DDEV_RES res;
    DDEV_REQ req;
    W rsize, errno, win;
    REQ_LIST *list, *new;

    win = packet->body.prd_req.dd;
    if (cmsg != NULL || (win != focus)) {
	if (!(key_mode & WAITMODE)) {
	    pres.receive_port = 0;
	    pres.msg_length = sizeof(pres);
	    pres.operation = PSC_READ;
	    pres.errno = EP_OK;
	    packet->body.prd_req.buf[0] = 0;
	    pres.status = 1;
	    pres.ret1 = 0;
	    pres.ret2 = 0;
	    snd_mbf(caller, sizeof(pres), &pres);
	    return (E_OK);
	}
	/* packet を win の queue に登録 */
	list = w[win].list;
	new = free_req_list;
	if (new == NULL) {
	    dbg_printf("WCONSOLE: emtpy request list\n");
	    return (E_NOMEM);
	}
	free_req_list = new->next;
	bcopy(packet, &(new->msg), sizeof(DDEV_REQ));

	if (list == NULL) {
	    w[win].list = new;
	    new->next = new;
	    new->prev = new;
	} else {
	    new->next = list;
	    new->prev = list->prev;
	    list->prev->next = new;
	    list->prev = new;
	}
    } else {
	/* cmsg へ packet を登録 */
	cmsg = free_req_list;
	if (cmsg == NULL) {
	    dbg_printf("WCONSOLE: emtpy request list\n");
	    return (E_NOMEM);
	}
	free_req_list = cmsg->next;
	cmsg->next = NULL;
	bcopy(packet, &(cmsg->msg), sizeof(DDEV_REQ));

	/* keyboard へ read を発行 */
	req.header.mbfid = recvport;
	req.header.msgtyp = DEV_RLY;
	req.body.rea_req.dd = focus;
	req.body.rea_req.start = packet->body.prd_req.start;
	req.body.rea_req.size = packet->body.prd_req.length;
	errno = snd_mbf(keyboard, sizeof(req), &req);

	/* 送信に失敗した場合 */
	if (errno != E_OK) {
	    dbg_printf("WCONSOLE: cant send message to keyboard\n");
	}

	return (errno);
    }
    return (E_OK);
}

/************************************************************************
 * write_wconsole
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	
 *
 * 処理：	カレントポインタの位置に文字列を表示する。
 *		このとき、エスケープシーケンスによる処理も行う。		
 *
 */
W write_wconsole(ID caller, DDEV_WRI_REQ * packet)
{
    DDEV_RES res;
    ER error;
    DDEV_REQ req;		/* 要求パケット */
    W rsize;

	req.header.mbfid = recv;
	req.header.msgtyp = DEV_WRI;
	bcopy(packet, &(req.body.ctl_req), sizeof(req.body));
	error = snd_mbf(console, sizeof(req), &req);
	if (error != E_OK) {
	    res.body.ctl_res.dd = packet->dd;
	    res.body.ctl_res.errcd = error;
	    res.body.ctl_res.errinfo = error;
	    snd_mbf(caller, sizeof(res), &res);
	    return (error);
	}
	rsize = sizeof(res);
	error = rcv_mbf(&res, (INT *) & rsize, recv);

	res.body.ctl_res.dd = packet->dd;
	res.body.ctl_res.errcd = E_OK;
	res.body.ctl_res.errinfo = E_OK;
	snd_mbf(caller, sizeof(res), &res);
	return (E_OK);
}

/************************************************************************
 * control_wconsole
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	
 *
 * 処理：画面のモードの変更、グラフィックスの描画、ウィンドウの操作など
 *
 */

static void respond_ctrl(ID caller, W dd, ER errno)
{
    DDEV_RES res;

    res.body.ctl_res.dd = dd;
    res.body.ctl_res.errcd = errno;
    res.body.ctl_res.errinfo = errno;
    snd_mbf(caller, sizeof(res), &res);
}

W control_wconsole(ID caller, ID tskid, DDEV_CTL_REQ * packet)
{
    DDEV_REQ req;		/* 要求パケット */
    DDEV_RES res;
    ER errno;
    W rsize;
    W x, y;
    union wc_cmd_t *p;
    int redraw;

	switch (packet->cmd) {
	case CONSOLE_CLEAR:
	    clear_wconsole(0);
	    respond_ctrl(caller, packet->dd, E_OK);
	    return (E_OK);

	case CONSOLE_MOVE:
	    if (packet->len != 2) {
		respond_ctrl(caller, packet->dd, E_PAR);
		return (E_PAR);
	    }
	    set_curpos(packet->param[0], packet->param[1], 0);
	    respond_ctrl(caller, packet->dd, E_OK);
	    return (E_OK);

	case KEYBOARD_ENA_EOF:
	    key_mode |= ENAEOFMODE;
	    respond_ctrl(caller, packet->dd, E_OK);
	    return (E_OK);

	case KEYBOARD_DIS_EOF:
	    key_mode &= ~ENAEOFMODE;
	    respond_ctrl(caller, packet->dd, E_OK);
	    return (E_OK);

	case KEYBOARD_CHANGEMODE:
	    key_mode = packet->param[0];

	    /* keyboard driver に転送 */
	    req.header.mbfid = recv;
	    req.header.msgtyp = DEV_CTL;
	    req.body.ctl_req.cmd = packet->cmd;
	    req.body.ctl_req.param[0] = packet->param[0];
	    errno = snd_mbf(keyboard, sizeof(req), &req);
	    if (errno != E_OK) {
		respond_ctrl(caller, packet->dd, errno);
		return (errno);
	    }
	    rsize = sizeof(res);
	    errno = rcv_mbf(&res, (INT *) & rsize, recv);

	    respond_ctrl(caller, packet->dd, E_OK);
	    return (E_OK);

	case KEYBOARD_GETMODE:
	    vput_reg(tskid, (VP) (((W *) packet->param)[0]),
		     sizeof(W), &key_mode);
	    respond_ctrl(caller, packet->dd, E_OK);
	    return (E_OK);

	default:
	    respond_ctrl(caller, packet->dd, E_NOSPT);
	    return (E_NOSPT);
	}
}


/* relay_wconsoe()
 */
W relay_wconsole(ID caller, DDEV_RLY_REQ * packet)
{
    struct posix_response pres;
    DDEV_REQ req;
    ER errno;

    /* keyboard からの返事の処理 */
    if (cmsg != NULL) {
	/* POSIX アプリケーション に返事を発行 */
	vput_reg(cmsg->msg.header.tskid,
		 cmsg->msg.body.prd_req.buf,
		 cmsg->msg.body.prd_req.length, packet->dt);

	pres.receive_port = 0;
	pres.msg_length = sizeof(pres);
	pres.operation = PSC_READ;
	pres.errno = EP_OK;
	if ((key_mode & ENAEOFMODE) && (packet->dt[0] == C('d'))) {
	    pres.status = 0;
	} else {
	    pres.status = cmsg->msg.body.prd_req.length;
	}
	pres.ret1 = 0;
	pres.ret2 = 0;
	snd_mbf(cmsg->msg.header.mbfid, sizeof(pres), &pres);

	/* cmsg の消去 */
	cmsg->next = free_req_list;
	free_req_list = cmsg;
	cmsg = NULL;
    }
}
