/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/keyboard/main.c,v 1.7 2000/04/03 14:30:05 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/keyboard/main.c,v 1.7 2000/04/03 14:30:05 naniwa Exp $";

/*
 * $Log: main.c,v $
 * Revision 1.7  2000/04/03 14:30:05  naniwa
 * to call timer handler in task
 *
 * Revision 1.6  2000/02/27 15:28:43  naniwa
 * to work as multi task OS
 *
 * Revision 1.5  2000/01/30 19:02:53  kishida0
 * add toplevel Makefile
 * use same keyboard keytop
 *   thanks nakanishi
 * now testing a scroll command (buggy!!)
 *
 * Revision 1.4  2000/01/24 23:28:52  kishida0
 * JIS -> EUC kanji code convert
 *
 * Revision 1.3  2000/01/23 15:50:49  kishida0
 * 106JP、101US キーボード切替え用のコマンドを追加した。
 * 関数にしているのがいまいちだが、とりあえずは使用可能
 * note-pc用にはもう少しキーのテーブルを確認する必要がある
 * 要レポート
 *
 * Revision 1.2  1999/03/15 05:58:27  monaka
 * modified some debug messages.
 *
 * Revision 1.1  1996/07/24 16:28:44  night
 * IBM PC 版 B-Free OS への最初の登録
 *
 * Revision 1.4  1995/10/01  12:55:24  night
 * KEYBOARD_CHANGEMODE についての処理を追加。
 *
 * Revision 1.3  1995/09/21  15:51:00  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.2  1995/09/19  18:01:02  night
 * キーボードから入力できるところまでできた。
 *
 * Revision 1.1  1995/09/18  11:42:02  night
 * 最初の登録
 *
 *
 *
 */

/**************************************************************************
 Discription

**********************************************************************/

#include "../../lib/libkernel/libkernel.h"
#include "keyboard.h"
#include "key_type.h"

/*********************************************************************
 *	 局所変数群の宣言
 *
 */
static void main_loop();
static void doit(DDEV_REQ * packet);
W send_dbg_msg();

/*********************************************************************
 *	 大域変数群の宣言
 *
 */
ID recvport;
W initialized = 0;
ID waitflag;			/* キーボードからキー入力を待つ時に */
				/* 使用するイベントフラグの ID */
W driver_mode;
ID local_recv = 0;
ID my_tskid;


/*
 * keyboard デバイスドライバの main 関数
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
    init_keyboard();

    /*
     * 立ち上げメッセージ
     */
    dbg_printf("keyboard driver started. receive port is %d\n", recvport);

    main_loop();
}

static void main_loop()
{
    DDEV_REQ req;		/* 受信する要求パケット */
    ER errno;
    INT rsize;

    /*
     * 要求受信 - 処理のループ
     */
    rsize = sizeof(req);
    for (;;) {

	/* 要求の受信 */
#ifdef undef
	errno = trcv_mbf(&req, &rsize, recvport, 51);
#else
	errno = rcv_mbf(&req, &rsize, recvport);
#endif
	switch (errno) {
	case E_OK:
	    /* 正常ケース */
	    doit(&req);
	    break;

	case E_TMOUT:
	case E_RLWAI:
	    break;

	default:
	    /* Unknown error */
	    dbg_printf("KEYBOARD: get_req() Unknown error(error = %d)\n",
		       errno);
	    break;
	}
    }

    /* ここの行には、来ない */
}

/*
 * 初期化
 *
 * o 要求受けつけ用のメッセージバッファ ID をポートマネージャに登録
 */
W init_keyboard(void)
{
    int i;
    ER error;

    /*
     * 要求受けつけ用のポートを初期化する。
     */
#ifdef notdef
    recvport = get_port(sizeof(DDEV_REQ), sizeof(DDEV_REQ));
#else
    recvport = get_port(0, sizeof(DDEV_REQ));
#endif
    if (recvport <= 0) {
	dbg_printf("KEYBOARD: cannot make receive port.\n");
	slp_tsk();
	/* メッセージバッファ生成に失敗 */
    }

    error = regist_port(KEYBOARD_DRIVER, recvport);
    if (error != E_OK) {
	dbg_printf("keyboard: cannot regist port (error = %d)\n", error);
    }

    init_keyboard_interrupt();	/* 割り込みハンドラの登録 */
    init_keybuffer();		/* キーボードバッファの初期化 */

    /* キー入力を待つ時に使用するイベントフラグの初期化 */
    waitflag = get_flag(TA_WSGL, 0);
    dbg_printf("keyboard: eventflag = %d\n", waitflag);	/* */

    driver_mode = WAITMODE | ENAEOFMODE;

    initialized = 1;

    get_tid(&my_tskid);
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
	    init_keyboard();
	}
	open_keyboard(packet->header.mbfid, &(packet->body.opn_req));
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_keyboard(packet->header.mbfid, &(packet->body.cls_req));
	break;

    case DEV_REA:
	read_keyboard(packet->header.mbfid, &(packet->body.rea_req));
	break;

    case DEV_PRD:
	posix_read_keyboard(packet->header.mbfid, packet->header.tskid,
			    &(packet->body.prd_req));
	break;

    case DEV_RLY:
	relay_keyboard(packet->header.mbfid, &(packet->body.rly_req));
	break;

    case DEV_WRI:
	write_keyboard(packet->header.mbfid, &(packet->body.wri_req));
	break;

    case DEV_CTL:
	control_keyboard(packet->header.mbfid, packet->header.tskid,
			 &(packet->body.ctl_req));
	break;
    }
}

/************************************************************************
 * open_keyboard --- keyboard のオープン
 *
 * 引数：	caller	メッセージの送り手
 *		packet	オープンパケット
 *
 * 返値：	常に E_OK を返す。
 *
 * 処理：	E_OK をメッセージの送り手に返す。
 *
 */
W open_keyboard(ID caller, DDEV_OPN_REQ * packet)
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
 * close_keyboard --- ドライバのクローズ
 *
 * 引数：	dd	keyboard ドライバ番号
 *		o_mode	オープンモード
 *		error	エラー番号
 *
 * 返値：	常に E_OK を返す。
 *
 * 処理：	キーボードはクローズの処理ではなにもしない。
 *
 */
W close_keyboard(ID caller, DDEV_CLS_REQ * packet)
{
    DDEV_RES res;

    res.body.cls_res.dd = packet->dd;
    res.body.cls_res.errcd = E_OK;
    res.body.cls_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/*************************************************************************
 * read_keyboard --- 
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	E_OK を返す。
 *
 * 処理：	メッセージの送り手に読み込んだ文字列を返す。
 *
 */
W read_keyboard(ID caller, DDEV_REA_REQ * packet)
{
    DDEV_RES res;
    W i;

    res.body.rea_res.dd = packet->dd;
    for (i = 0; i < packet->size; i++) {
	res.body.rea_res.dt[i] = read_key(driver_mode);
    }
    if ((driver_mode & ENAEOFMODE) && (packet->size == 1) &&
	(res.body.rea_res.dt[0] == C('d'))) {
	res.body.rea_res.a_size = 0;
    } else {
	res.body.rea_res.a_size = i;
    }
    res.body.rea_res.errcd = E_OK;
    res.body.rea_res.errinfo = E_OK;

#if 0
    dbg_printf("KEYBOARD: send to caller\n");
#endif
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/*************************************************************************
 * posix_read_keyboard --- 
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	E_OK を返す。
 *
 * 処理：	メッセージの送り手(POSIX lowlib)に文字列を返す。
 *
 */
W posix_read_keyboard(ID caller, ID tskid, DDEV_PRD_REQ * packet)
{
    struct posix_response res;
    W i;

#ifdef notdef
    dbg_printf("\nKEYBOARD: posix_read_keyboard %d len = %d\n",
	       caller, packet->length);
#endif
    {
	B buf[packet->length + 1];

	for (i = 0; i < packet->length; i++) {
	    buf[i] = read_key(driver_mode);
	}
	buf[packet->length] = 0;
	if ((driver_mode & ENAEOFMODE) && (packet->length == 1) &&
	    (buf[0] == C('d'))) {
	    i = 0;
	} else {
	    i = packet->length;
	}
	vput_reg(tskid, packet->buf, i, buf);

	res.receive_port = 0;
	res.msg_length = sizeof(res);
	res.operation = PSC_READ;
	res.errno = EP_OK;
	res.status = packet->length;
	res.ret1 = 0;
	res.ret2 = 0;

#ifdef notdef
	dbg_printf("KEYBOARD: reply to app %d\n", caller);
#endif
	snd_mbf(caller, sizeof(res), &res);
	return (E_OK);
    }
}

/************************************************************************
 * write_keyboard
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	常に E_NOSPT を返す。
 *
 * 処理：	write は、キーボードでは行わない。
 *
 */
W write_keyboard(ID caller, DDEV_WRI_REQ * packet)
{
    DDEV_RES res;

    res.body.rea_res.dd = packet->dd;
    res.body.rea_res.errcd = E_NOSPT;
    res.body.rea_res.errinfo = E_NOSPT;
    snd_mbf(caller, sizeof(res), &res);
    return (E_NOSPT);
}

/************************************************************************
 * control_keyboard
 *
 * 引数：	caller
 *		packet
 *
 * 返値：	E_NOSPT を返す。
 *
 * 処理：
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

W control_keyboard(ID caller, ID tskid, DDEV_CTL_REQ * packet)
{
    switch (packet->cmd) {
    case KEYBOARD_CLEAR:
	clear_keybuffer();
	respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    case KEYBOARD_CHANGEMODE:
        driver_mode = ((W *) packet->param)[0];
#ifdef notdef
	dbg_printf("KEYBOARD: new keyboard mode %d\n", driver_mode);
#endif
	respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    case KEYBOARD_CHANGE_106JP:
	keyboard_select = K_106JP;
	respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    case KEYBOARD_CHANGE_101US:
	keyboard_select = K_101US;
	respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    case KEYBOARD_ENA_EOF:
	driver_mode |= ENAEOFMODE;
	respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    case KEYBOARD_DIS_EOF:
        driver_mode &= ~ENAEOFMODE;
	respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    case KEYBOARD_GETMODE:
        vput_reg(tskid, ((W *) packet->param)[0], sizeof(W), &driver_mode);
        respond_ctrl(caller, packet->dd, E_OK);
	return (E_OK);

    default:
	respond_ctrl(caller, packet->dd, E_NOSPT);
	return (E_NOSPT);
    }
}

/* relay_keyboard()
 */
W relay_keyboard(ID caller, DDEV_RLY_REQ * packet)
{
    DDEV_REQ req;
    W i;

    req.header.mbfid = local_recv;
    req.header.msgtyp = DEV_RLY;
    req.body.rly_req.dd = packet->dd;
#ifdef notdef
    dbg_printf("KEYBOARD: read %d\n", packet->size);
#endif
    for (i = 0; i < packet->size; i++) {
	req.body.rly_req.dt[i] = read_key(driver_mode);
    }
    req.body.rly_req.size = i;

#ifdef notdef
    dbg_printf("KEYBORAD: send relay to wconsole\n");
#endif
    snd_mbf(caller, sizeof(req), &req);
    return (E_OK);
}
