/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
#include "../../lib/libserv/libserv.h"
#include "../../include/itron/rendezvous.h"
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
static void doit(RDVNO rdvno, devmsg_t * packet);

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
    dbg_printf("[RAMDISK] started. port = %d, size = %d KB\n",
	    recvport, size / 1024);

    /*
     * ドライバを初期化する。
     */
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
	    dbg_printf("[RAMDISK] acp_por error = %d\n",
		       rsize);
	    dbg_printf("[RAMDISK] halt.\n");
	    ext_tsk();
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
static void doit(RDVNO rdvno, devmsg_t * packet)
{
    switch (packet->req.header.msgtyp) {
    case DEV_OPN:
	/* デバイスのオープン */
	open_rd(rdvno, packet);
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_rd(rdvno, packet);
	break;

    case DEV_REA:
	read_rd(rdvno, packet);
	break;

    case DEV_WRI:
	write_rd(rdvno, packet);
	break;

    case DEV_CTL:
	control_rd(rdvno, packet);
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
    T_CPOR pk_cpor = { TA_TFIFO, sizeof(DDEV_REQ), sizeof(DDEV_RES) };

    /*
     * 要求受けつけ用のポートを初期化する。
     */
    recvport = acre_por(&pk_cpor);

    if (recvport <= 0) {
	dbg_printf("[RAMDISK] cre_por error = %d\n", recvport);
	ext_tsk();
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
	dbg_printf("[RAMDISK] cannot allocate memory.\n");
	_exit(-1);
    } else {
	ramdisk = (char *) VADDR_HEAP;
    }
#else
    init_malloc(VADDR_HEAP);	/* 適当な値 */
    ramdisk = (char *) malloc(size);
    if (ramdisk == NULL) {
	dbg_printf("[RAMDISK] cannot allocate memory.\n");
	ext_tsk();
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
W open_rd(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_OPN_REQ * req = &(packet->req.body.opn_req);
    DDEV_OPN_RES * res = &(packet->res.body.opn_res);

    res->dd = req->dd;
    res->size = rd_size;
    res->errcd = E_OK;
    res->errinfo = E_OK;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
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
W close_rd(RDVNO rdvno, devmsg_t * packet)
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
W read_rd(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_REA_REQ * req = &(packet->req.body.rea_req);
    DDEV_REA_RES * res = &(packet->res.body.rea_res);
    W done_length;

    if (req->start + req->size >= rd_size)
	done_length = rd_size - req->start;
    else
	done_length = req->size;

    bcopy(&ramdisk[req->start], res->dt, done_length);
    res->dd = req->dd;
    res->a_size = done_length;
    res->errcd = E_OK;
    res->errinfo = E_OK;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    return (E_OK);
}


/************************************************************************
 *	write_rd
 */
W write_rd(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_WRI_REQ * req = &(packet->req.body.wri_req);
    DDEV_WRI_RES * res = &(packet->res.body.wri_res);
    W done_length;

    if (req->start + req->size >= rd_size)
	done_length = rd_size - req->start;
    else
	done_length = req->size;

    bcopy(req->dt, &ramdisk[req->start], done_length);

    res->dd = req->dd;
    res->a_size = done_length;
    res->errcd = E_OK;
    res->errinfo = E_OK;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    return (E_OK);
}

/************************************************************************
 *	control_rd
 */
W control_rd(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_CTL_REQ * req = &(packet->req.body.ctl_req);
    DDEV_CTL_RES * res = &(packet->res.body.ctl_res);
    ER error = E_OK;
    W drive;

    switch (req->cmd) {
    default:
	error = E_NOSPT;
	break;
    }
    res->dd = req->dd;
    res->errcd = error;
    res->errinfo = error;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    return (error);
}
