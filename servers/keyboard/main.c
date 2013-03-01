/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/keyboard/main.c,v 1.7 2000/04/03 14:30:05 naniwa Exp $ */

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

#include <device.h>
#include <itron/rendezvous.h>
#include "../../lib/libserv/libserv.h"
#include "../../lib/libserv/port.h"
#include "keyboard.h"
#include "key_type.h"

/*********************************************************************
 *	 局所変数群の宣言
 *
 */
static void main_loop();
static void doit(RDVNO rdvno, devmsg_t * packet);
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
void start()
{
    /* 
     * 要求受信用のポートの作成
     */
    init_keyboard();

    /*
     * 立ち上げメッセージ
     */
    dbg_printf("[KEYBOARD] started. port = %d\n", recvport);

    main_loop();
}

static void main_loop()
{
    /*
     * 要求受信 - 処理のループ
     */
    for (;;) {
	devmsg_t packet;
	ER_UINT rsize;
	RDVNO rdvno;

	/* 要求の受信 */
	rsize = acp_por(recvport, 0xffffffff, &rdvno, &packet);

	if (rsize >= 0) {
	    /* 正常ケース */
	    doit(rdvno, &packet);
	}

	else {
	    /* Unknown error */
	    dbg_printf("[KEYBOARD] acp_por error = %d\n",
		       rsize);
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
    ER error;
    T_CPOR pk_cpor = { TA_TFIFO, sizeof(DDEV_REQ), sizeof(DDEV_RES) };
    T_CFLG pk_cflg = { NULL, TA_WSGL, 0 };

    /*
     * 要求受けつけ用のポートを初期化する。
     */
    recvport = acre_por(&pk_cpor);

    if (recvport <= 0) {
	dbg_printf("[KEYBOARD] acre_por error = \n", recvport);
	ext_tsk();
	/* メッセージバッファ生成に失敗 */
    }

    error = regist_port((port_name*)KEYBOARD_DRIVER, recvport);
    if (error != E_OK) {
	dbg_printf("[KEYBOARD] cannot regist port. error = %d\n", error);
	ext_tsk();
    }

    init_keyboard_interrupt();	/* 割り込みハンドラの登録 */
    init_keybuffer();		/* キーボードバッファの初期化 */

    /* キー入力を待つ時に使用するイベントフラグの初期化 */
    waitflag = acre_flg(&pk_cflg);
    dbg_printf("[KEYBOARD] eventflag = %d\n", waitflag);	/* */

    driver_mode = WAITMODE | ENAEOFMODE;

    initialized = 1;

    get_tid(&my_tskid);

    return E_OK;
}

/************************************************************************
 *
 *
 */
static void doit(RDVNO rdvno, devmsg_t * packet)
{
    switch (packet->req.header.msgtyp) {
    case DEV_OPN:
	/* デバイスのオープン */
	if (!initialized) {
	    init_keyboard();
	}
	open_keyboard(rdvno, packet);
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_keyboard(rdvno, packet);
	break;

    case DEV_REA:
	read_keyboard(rdvno, packet);
	break;

    case DEV_WRI:
	write_keyboard(rdvno, packet);
	break;

    case DEV_CTL:
	control_keyboard(rdvno, packet);
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
W open_keyboard(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_OPN_REQ * req = &(packet->req.body.opn_req);
    DDEV_OPN_RES * res = &(packet->res.body.opn_res);

    res->dd = req->dd;
    res->size = 0;
    res->errcd = E_OK;
    res->errinfo = E_OK;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
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
W close_keyboard(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_CLS_REQ * req = &(packet->req.body.cls_req);
    DDEV_CLS_RES * res = &(packet->res.body.cls_res);

    res->dd = req->dd;
    res->errcd = E_OK;
    res->errinfo = E_OK;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
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
W read_keyboard(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_REA_REQ * req = &(packet->req.body.rea_req);
    DDEV_REA_RES * res = &(packet->res.body.rea_res);
    W i;
    ER error;

    res->dd = req->dd;
    for (i = 0; i < req->size; i++) {
	res->dt[i] = read_key(driver_mode);
    }
    if ((driver_mode & ENAEOFMODE) && (req->size == 1) &&
	(res->dt[0] == C('d'))) {
	res->a_size = 0;
    } else {
	res->a_size = i;
    }
    res->errcd = E_OK;
    res->errinfo = E_OK;

#if 0
    dbg_printf("[KEYBOARD] reply to caller\n");
#endif
    error = rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    if (error) {
	dbg_printf("[KEYBOARD] read_keyboard rpl_rdv(%d) = %d\n", rdvno, error);
    }
    return (E_OK);
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
W write_keyboard(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_WRI_REQ * req = &(packet->req.body.wri_req);
    DDEV_WRI_RES * res = &(packet->res.body.wri_res);

    res->dd = req->dd;
    res->errcd = E_NOSPT;
    res->errinfo = E_NOSPT;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
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

static void respond_ctrl(RDVNO rdvno, devmsg_t * packet, W dd, ER errno)
{
    DDEV_CTL_RES * res = &(packet->res.body.ctl_res);

    res->dd = dd;
    res->errcd = errno;
    res->errinfo = errno;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
}

W control_keyboard(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_CTL_REQ * req = &(packet->req.body.ctl_req);

    switch (req->cmd) {
    case KEYBOARD_CLEAR:
	clear_keybuffer();
	respond_ctrl(rdvno, packet, req->dd, E_OK);
	return (E_OK);

    case KEYBOARD_CHANGEMODE:
	{
	    W *p = (W*)(req->param);
	    driver_mode = *p;
	}
#ifdef notdef
	dbg_printf("[KEYBOARD] respond_ctrl new keyboard mode %d\n",
		driver_mode);
#endif
	respond_ctrl(rdvno, packet, req->dd, E_OK);
	return (E_OK);

    case KEYBOARD_CHANGE_106JP:
	respond_ctrl(rdvno, packet, req->dd, E_NOSPT);
	return (E_NOSPT);

    case KEYBOARD_CHANGE_101US:
	respond_ctrl(rdvno, packet, req->dd, E_NOSPT);
	return (E_NOSPT);

    case KEYBOARD_ENA_EOF:
	driver_mode |= ENAEOFMODE;
	respond_ctrl(rdvno, packet, req->dd, E_OK);
	return (E_OK);

    case KEYBOARD_DIS_EOF:
        driver_mode &= ~ENAEOFMODE;
	respond_ctrl(rdvno, packet, req->dd, E_OK);
	return (E_OK);

    case KEYBOARD_GETMODE:
        vput_reg(packet->req.header.tskid, (VP)(req->param),
                sizeof(W),
                &driver_mode);
        respond_ctrl(rdvno, packet, req->dd, E_OK);
	return (E_OK);

    default:
	respond_ctrl(rdvno, packet, req->dd, E_NOSPT);
	return (E_NOSPT);
    }
}
