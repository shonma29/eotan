/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#include "ramdisk.h"

#define START_FROM_INIT

/*********************************************************************
 *	 大域変数群の宣言
 *
 */

/*********************************************************************
 *	 局所変数群の宣言
 *
 */
static W mydevid;		/* 自分自身のid */
static ID recvport;		/* 要求受けつけ用ポート */
static char *ramdisk;
static W rd_size;

/*
 *	局所関数群の定義
 */
static void main_loop(void);
static void init_rd_driver(W size);
static void init_rd(W size);
static void doit(DDEV_REQ * packet);

#ifdef START_FROM_INIT
/* ダミーの main 関数 */
main()
{
}

#endif
/*
 * デバイスドライバの main 関数
 *
 * この関数は、デバイスドライバ立ち上げ時に一回だけ実行する。
 *
 */

start(int argc, char *argv[])
{
    W size;
    extern int atoi(char *);

    if (argc < 2) {
	size = RD_SIZE;
    } else {
	size = atoi(argv[1]);
	if (size < 0)
	    size = RD_SIZE;
	else
	    size *= 1024;
    }
    /* 
     * 要求受信用のポートの作成とラムディスクの初期化
     */
    init_rd_driver(size);

    /*
     * 立ち上げメッセージ
     */
    dbg_printf("ram disk driver start (size = %d KB)\n", size / 1024);
    dbg_printf("  receive port is %d\n", recvport);

    /*
     * ドライバを初期化する。
     */
    main_loop();
}

static void main_loop()
{
    DDEV_REQ req;
    extern ER sys_errno;
    UW rsize;

    /*
     * 要求受信 - 処理のループ
     */
    for (;;) {
	/* 要求の受信 */
	get_req(recvport, &req, &rsize);
	switch (sys_errno) {
	case E_OK:
	    /* 正常ケース */
	    doit(&req);
	    break;

	default:
	    /* Unknown error */
	    dbg_printf("rd: get_req() Unknown error(error = %d)\n",
		       sys_errno);
	    dbg_printf("RAM DISK driver is halt.\n");
	    slp_tsk();
	    break;
	}
    }
    /* ここの行には、来ない */
}

/*
 *
 */
/************************************************************************
 *
 *
 */
static void doit(DDEV_REQ * packet)
{
    switch (packet->header.msgtyp) {
    case DEV_OPN:
	/* デバイスのオープン */
	open_rd(packet->header.mbfid, &(packet->body.opn_req));
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_rd(packet->header.mbfid, &(packet->body.cls_req));
	break;

    case DEV_REA:
	read_rd(packet->header.mbfid, &(packet->body.rea_req));
	break;

    case DEV_WRI:
	write_rd(packet->header.mbfid, &(packet->body.wri_req));
	break;

    case DEV_CTL:
	control_rd(packet->header.mbfid, &(packet->body.ctl_req));
	break;
    }
}

/*
 * 初期化
 *
 * o ファイルテーブル (file_table) の初期化
 * o 要求受けつけ用のメッセージバッファ ID をポートマネージャに登録
 */
static void init_rd_driver(W size)
{
    int i;
    ID root_dev;
    ER error;

    /*
     * 要求受けつけ用のポートを初期化する。
     */
#ifdef notdef
    recvport = get_port(sizeof(DDEV_RES), sizeof(DDEV_RES));
#else
    recvport = get_port(0, sizeof(DDEV_RES));
#endif
    if (recvport <= 0) {
	dbg_printf("RD: cannot make receive porrt.\n");
	slp_tsk();
	/* メッセージバッファ生成に失敗 */
    }

    error = regist_port(RD_DRIVER, recvport);
    if (error != E_OK) {
	/* error */
    }

    init_rd(size);
}

/*
 * init_rd --- RAM DISK ドライバの初期化
 *
 */
void init_rd(W size)
{
#ifdef START_FROM_INIT
    if (brk(VADDR_HEAP + size) < 0) {
	dbg_printf("RD: cannot allocate memory.\n");
	_exit(-1);
    } else {
	ramdisk = (char *) VADDR_HEAP;
    }
#else
    init_malloc(VADDR_HEAP);	/* 適当な値 */
    ramdisk = (char *) malloc(size);
    if (ramdisk == NULL) {
	dbg_printf("RD: cannot allocate memory.\n");
	slp_tsk();
    }
#endif
    rd_size = size;
}

/************************************************************************
 * open_rd --- RAM DISK のオープン
 *
 * 引数：	dd	ドライバ番号
 *		o_mode	オープンモード
 *		error	エラー番号
 *
 * 返値：	
 *
 *
 */
W open_rd(ID caller, DDEV_OPN_REQ * packet)
{
    DDEV_RES res;

    res.body.opn_res.dd = packet->dd;
    res.body.opn_res.size = rd_size;
    res.body.opn_res.errcd = E_OK;
    res.body.opn_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/************************************************************************
 * close_rd --- ドライバのクローズ
 *
 * 引数：	dd	ドライバ番号
 *		o_mode	オープンモード
 *		error	エラー番号
 *
 * 返値：	
 *
 */
W close_rd(ID caller, DDEV_CLS_REQ * packet)
{
    DDEV_RES res;

    res.body.cls_res.dd = packet->dd;
    res.body.cls_res.errcd = E_OK;
    res.body.cls_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/*************************************************************************
 * read_rd --- 
 *
 * 引数：	caller	呼び出し元への返答を返すためのポート
 *		packet	読み込みデータのパラメータ
 *
 * 返値：	エラー番号
 *
 * 処理：	この関数は、以下の処理を行う。
 *
 */
W read_rd(ID caller, DDEV_REA_REQ * packet)
{
    DDEV_RES res;
    W done_length;

    if (packet->start + packet->size >= rd_size)
	done_length = rd_size - packet->start;
    else
	done_length = packet->size;

    bcopy(&ramdisk[packet->start], res.body.rea_res.dt, done_length);
    res.body.rea_res.dd = packet->dd;
    res.body.rea_res.a_size = done_length;
    res.body.rea_res.errcd = E_OK;
    res.body.rea_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}


/************************************************************************
 *	write_rd
 */
W write_rd(ID caller, DDEV_WRI_REQ * packet)
{
    DDEV_RES res;
    W done_length;

    if (packet->start + packet->size >= rd_size)
	done_length = rd_size - packet->start;
    else
	done_length = packet->size;

    bcopy(packet->dt, &ramdisk[packet->start], done_length);

    res.body.wri_res.dd = packet->dd;
    res.body.wri_res.a_size = done_length;
    res.body.wri_res.errcd = E_OK;
    res.body.wri_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
    return (E_OK);
}

/************************************************************************
 *	control_rd
 */
W control_rd(ID caller, DDEV_CTL_REQ * packet)
{
    DDEV_RES res;
    ER error = E_OK;
    W drive;

    switch (packet->cmd) {
    default:
	error = E_NOSPT;
	break;
    }
    res.body.ctl_res.dd = packet->dd;
    res.body.ctl_res.errcd = error;
    res.body.ctl_res.errinfo = error;
    snd_mbf(caller, sizeof(res), &res);
    return (error);
}
