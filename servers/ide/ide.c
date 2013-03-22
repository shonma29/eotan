/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/ide.c,v 1.15 2000/01/22 10:59:30 naniwa Exp $ */

/*
 * $Log: ide.c,v $
 * Revision 1.15  2000/01/22 10:59:30  naniwa
 * fixed to be able to write disk
 *
 * Revision 1.14  1999/04/13 04:14:43  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.13  1999/03/15 05:58:23  monaka
 * modified some debug messages.
 *
 * Revision 1.12  1998/06/10 15:46:24  night
 * パーティション情報として、CHS ではなくリニアブロック番号を使うように
 * 変更した。
 *
 * Revision 1.11  1998/05/07 16:47:42  night
 * パーティションの最初のブロック番号を計算するときに、
 * 拡張パーティションのことを考慮するようにした。
 *
 * Revision 1.10  1998/02/23 14:39:38  night
 * get_req の引数が間違っていたため修正。
 * 今までのコードだと、仮想空間の先頭部分を破壊していた。
 *
 * Revision 1.9  1997/10/11 16:18:51  night
 * IDE HD への書き込み処理を正常に動くようにいろいろ修正。
 *
 * Revision 1.8  1997/07/07 14:43:14  night
 * IDE ドライバの write 処理もパーティションを意識するように変更した。
 *
 * Revision 1.7  1997/07/06 11:46:24  night
 * ブロックオフセットの計算方法を変更。
 *
 * Revision 1.6  1997/07/04 15:03:58  night
 * パーティションの指定方法を変更。
 * dd が 0 のときには、ディスク全体を示すようにした。
 * (従来は、dd が 0 のときにはパーティション 0 を示していた)
 * この変更により、最初のパーティションは、dd が 1 で指定することになる。
 *
 * Revision 1.5  1997/07/02 13:08:31  night
 * 割り込みを使わないように修正。
 *
 * Revision 1.4  1997/05/11 12:41:12  night
 * read/write 時にパーティション情報を考慮するように変更した。
 *
 * Revision 1.3  1997/05/08 15:07:12  night
 * IDE ドライブの情報を記録する領域を新しく設定した。
 *
 * Revision 1.2  1997/05/06 12:40:55  night
 * IDE_GET_STAT 追加。
 *
 * Revision 1.1  1996/11/05  18:42:35  night
 * 最初の登録
 *
 *
 *
 */

/*
 * IDE HD デバイスドライバ
 *
 *
 */
#include <string.h>
#include <itron/rendezvous.h>
#include <mpu/io.h>
#include <arch/8259a.h>
#include <arch/archfunc.h>
#include "../../lib/libserv/libserv.h"
#include "../../lib/libserv/port.h"
#include "ide.h"


/*********************************************************************
 *	 大域変数群の宣言
 *
 */
W ide_intr_flag;
struct ide_id ide_spec[MAXIDE];
struct ide_partition
 ide_partition[MAXIDE][IDE_MAX_PARTITION + IDE_MAX_EXT_PARTITION];

W ext_partition;


/*********************************************************************
 *	 局所変数群の宣言
 *
 */
static ID recvport;		/* 要求受けつけ用ポート */
static W initialized;

/*
 *	局所関数群の定義
 */
static void main_loop(void);
static int init_ide_driver(void);
static void doit(RDVNO rdvno, devmsg_t * packet);

/*
 * FD デバイスドライバの main 関数
 *
 * この関数は、デバイスドライバ立ち上げ時に一回だけ実行する。
 *
 */
void start()
{
    /* 
     * 要求受信用のポートの作成
     */
    if (init_ide_driver() != E_OK) {
	ext_tsk();
    }

    /*
     * 立ち上げメッセージ
     */
    dbg_printf("[IDE] started. port = %d\n", recvport);

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
/*      dbg_printf ("[IDE] acp_por\n"); */
	rsize = acp_por(recvport, 0xffffffff, &rdvno, &packet);

	if (rsize >= 0) {
	    /* 正常ケース */
#ifdef DEBUG
	    dbg_printf("[IDE] receive packet type = %d\n",
		    packet.req.header.msgtyp);
#endif
	    doit(rdvno, &packet);
	}

	else {
	    /* Unknown error */
	    dbg_printf("[IDE] acp_por error = %d\n",
		       rsize);
	    dbg_printf("[IDE] halt.\n");
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

    if (!initialized) {
	dbg_printf("[IDE] device is not initialized\n");
    }
    switch (packet->req.header.msgtyp) {
    case DEV_OPN:
	/* デバイスのオープン */
	open_ide(rdvno, packet);
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_ide(rdvno, packet);
	break;

    case DEV_REA:
	read_ide(rdvno, packet);
	break;

    case DEV_WRI:
	write_ide(rdvno, packet);
	break;

    case DEV_CTL:
	control_ide(rdvno, packet);
	break;
    }
}

/*
 * 初期化
 *
 * o ファイルテーブル (file_table) の初期化
 * o 要求受けつけ用のメッセージバッファ ID をポートマネージャに登録
 */
static int init_ide_driver(void)
{
    ER error;
    T_CPOR pk_cpor = { TA_TFIFO, sizeof(DDEV_REQ), sizeof(DDEV_RES) };

    initialized = 0;
    if (init_ide() != E_OK) {
	dbg_printf("[IDE] error on initialize\n");
	return E_SYS;
    }

    /*
     * 要求受けつけ用のポートを初期化する。
     */
    recvport = acre_por(&pk_cpor);

    if (recvport <= 0) {
	dbg_printf("[IDE] acre_por error = %d\n", recvport);
	ext_tsk();
	/* メッセージバッファ生成に失敗 */
    }

    error = regist_port((port_name*)IDE_DRIVER, recvport);
    if (error != E_OK) {
	dbg_printf("[IDE] cannot regist port. error = %d\n", error);
    }

    return E_OK;
}

/*
 * init_ide --- IDE ドライバの初期化
 *
 */
W init_ide(void)
{
    int status;

    outb(IDE_CONTROL_REG, IDE_SRST);	/* ソフトウェア・リセット */
    outb(IDE_CONTROL_REG, IDE_nIEN);	/* 割込み禁止 */
    status = ide_wait_while_busy();
    dbg_printf("[IDE] init status = 0x%x/0x%x\n",
	       inb(IDE_ERROR_REG), status);
    if (status == 0x50) {
	ide_recalibrate(0);		/* drive 0 の recalibrate */
	read_partition(0);
	return E_OK;
    }
    return E_SYS;
}

/************************************************************************
 * open_ide --- IDE のオープン
 *
 * 引数：	dd	IDE ドライバ番号
 *		o_mode	オープンモード
 *		error	エラー番号
 *
 * 返値：	
 *
 * 処理：	指定した IDE のステータスを調べ、もし使われていなければ、
 *		使用状態 (DRIVE_USING) にする。
 *
 */
W open_ide(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_OPN_REQ * req = &(packet->req.body.opn_req);
    DDEV_OPN_RES * res = &(packet->res.body.opn_res);
    W drive;			/* ドライブ番号 */
    UW partition;		/* パーティション番号 */

#ifdef notdef
    outb(0x439, (inb(0x439) & 0xfb));	/* DMA Accsess Control over 1MB */
    outb(0x29, (0x0c | 0));	/* Bank Mode Reg. 16M mode */
    outb(0x29, (0x0c | 1));	/* Bank Mode Reg. 16M mode */
    outb(0x29, (0x0c | 2));	/* Bank Mode Reg. 16M mode */
    outb(0x29, (0x0c | 3));	/* Bank Mode Reg. 16M mode */
#endif
    res->dd = req->dd;
    drive = IDE_GET_DRIVE(req->dd);
    partition = IDE_GET_PARTITION(req->dd);
    if ((partition > IDE_WHOLE_DISK) &&
	(partition <= (IDE_MAX_PARTITION + ext_partition))) {
	res->size =
	    ide_partition[drive][partition - 1].length * IDE_BLOCK_SIZE;
    } else if (partition == IDE_WHOLE_DISK) {
	res->size = ((UW) ide_spec[drive].now_sector) *
	    ((UW) ide_spec[drive].now_head) *
	    ((UW) ide_spec[drive].now_cylinder);
	if (res->size >= IDE_USABLE_SIZE) {
	    res->size = MAX_UINT;
	} else {
	    res->size *= IDE_BLOCK_SIZE;
	}
    } else {
	res->size = 0;
    }
#if 0
    dbg_printf("[IDE] open partition #%d size = %d\n", partition,
	       res->size);
#endif
    res->errcd = E_OK;
    res->errinfo = E_OK;
    rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    return (E_OK);
}

/************************************************************************
 * ide_close --- ドライバのクローズ
 *
 * 引数：	dd	IDE ドライバ番号
 *		o_mode	オープンモード
 *		error	エラー番号
 *
 * 返値：	
 *
 * 処理：	指定した IDE のステータスを調べ、もし使われていれば
 *		使用状態 (DRIVE_USING) のフラグを消す。
 *
 */
W close_ide(RDVNO rdvno, devmsg_t * packet)
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
 * read_ide --- 
 *
 * 引数：	caller	呼び出し元への返答を返すためのポート
 *		packet	読み込みデータのパラメータ
 *
 * 返値：	エラー番号
 *
 * 処理：	この関数は、以下の処理を行う。
 *
 *		1) 論理ブロック番号 (1024 byte 単位) から物理ブロック番号へ変換
 *		2) 物理ブロック番号からさらに Head/Cylinder/Sector の各々の数へ変換
 *		3) 変換した数を引数にして get_data() を呼び出す
 *
 */
W read_ide(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_REA_REQ * req = &(packet->req.body.rea_req);
    DDEV_REA_RES * res = &(packet->res.body.rea_res);
    W blockno;			/* 物理ブロック番号 */
    W bcount;			/* 論理ブロックが物理ブロックより大きい場合に使用する。 */
    /* 物理ブロックを読みとるときの回数となる              */
    W cylinder;
    W head;
    W sector;
    W drive;			/* ドライブ番号 */
    UW partition;		/* パーティション番号 */
    W i, try;
    W done_length = 0;		/* 本当に読み込んだバイト数 */
    static B buff[BLOCK_SIZE * 2];
    UW bp;
    ER ret;
    ER error;
    UW bufstart;
    UW buflength;
    UW parstart;
    ER reply_error;
    UW reqstart = req->start;
    UW reqsize = req->size;

    drive = IDE_GET_DRIVE(req->dd);
    partition = IDE_GET_PARTITION(req->dd);
    if (partition == IDE_WHOLE_DISK) {
	parstart = 0;
    }
    if ((drive < 0) ||
	(partition > (IDE_MAX_PARTITION + ext_partition))) {
	dbg_printf("[IDE] read_ide Illegal driver or partition number [%d/%d]\n",
		   drive, partition);
	goto bad;
    } else {
	parstart = ide_partition[drive][partition - 1].start * BLOCK_SIZE;
    }

    bufstart = ROUNDDOWN(reqstart, BLOCK_SIZE);
    buflength = ROUNDUP(reqstart + reqsize, BLOCK_SIZE);
#ifdef DEBUG
    dbg_printf("[IDE] read_ide partition = %d, bufstart = %d, buflength = %d\n",
	       partition, bufstart, buflength);	/* */
#endif

    error = E_OK;
    ret = E_OK;

    for (bp = 0; bp < (buflength - bufstart); bp += BLOCK_SIZE) {
#ifdef DEBUG
	dbg_printf("[IDE] read_ide bp = %d, length = %d\n", bp, BLOCK_SIZE);	/* */
#endif
	/* バイトオフセットから物理ブロック番号への変換 */
	blockno = ((UW) (bp + bufstart + parstart) / BLOCK_SIZE)
	    * (BLOCK_SIZE / IDE_BLOCK_SIZE);
	bcount = BLOCK_SIZE / IDE_BLOCK_SIZE;

#define H	(ide_spec[IDE_GET_DRIVE (req->dd)].n_head)
#define S	(ide_spec[IDE_GET_DRIVE (req->dd)].n_sector)
#define LEN	IDE_BLOCK_SIZE

#ifdef USE_LBA
	drive |= LBA_MODE_BIT;
#endif

	for (i = 0; i < bcount; i++) {
#ifndef USE_LBA
	    head = (blockno % (H * S)) / S;
	    cylinder = (blockno / (H * S));
	    sector = (blockno % S) + 1;
#else
	    sector = LBA_8(blockno);
	    cylinder = LBA_16(blockno);
	    head = LBA_4(blockno);
#endif

#ifdef DEBUG
	    dbg_printf("[IDE] read_ide (H = %d, C = %d, S = %d)\n", head,
		       cylinder, sector);	/* */
#endif
	    for (try = 0; try < IDE_RETRY; try++) {
#ifdef DEBUG
		dbg_printf("[IDE] read_ide get_data (%d, %d, %d, %d, 0x%x)\n",
			   drive, head, cylinder, sector,
			   (void *) &(buff[bp + (i * LEN)]));
#endif

		ret =
		    get_data(drive, head, cylinder, sector,
			     (void *) &(buff[bp + (i * LEN)]),
			     LEN / IDE_BLOCK_SIZE);
		if (ret > 0)
		    break;
	    }
	    if (ret > 0) {
		done_length += ret;
		blockno++;
	    } else {
		error = E_SYS;
		goto bad;
	    }
	}
    }

#ifdef DEBUG_1
    dbg_printf("[IDE] read_ide bcopy(0x%x, 0x%x, %d)\n",
	       diff,
	       res->dt,
	       (done_length < reqsize) ? done_length : reqsize);
#endif

    done_length =
	((done_length < reqsize) ? done_length : reqsize);
#ifdef notdef
    done_length -= reqstart - bufstart;
#endif
    if (done_length > BLOCK_SIZE * 2) {
	dbg_printf("[IDE] read_ide buffer overflow (size = %d)\n",
		   (done_length <
		    reqsize) ? done_length : reqsize);
	error = E_SYS;
	goto bad;
    }

    memcpy(res->dt,
	    &buff[reqstart - bufstart], done_length);
    res->dd = req->dd;
    res->a_size = done_length;
    res->errcd = E_OK;
    res->errinfo = 0;
    reply_error = rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    if (reply_error) {
	dbg_printf("[IDE] read_ide rpl_rdv error = %d\n", reply_error);
    }
    return (E_OK);

  bad:
    res->dd = req->dd;
    res->a_size = done_length;
    res->errcd = error;
    res->errinfo = error;
    reply_error = rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    if (reply_error) {
	dbg_printf("[IDE] read_ide rpl_rdv error = %d\n", reply_error);
    }
    return (error);

#undef H
#undef S
#undef LEN
}


/************************************************************************
 *	write_ide
 */
W write_ide(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_WRI_REQ * req = &(packet->req.body.wri_req);
    DDEV_WRI_RES * res = &(packet->res.body.wri_res);
    W blockno;			/* 物理ブロック番号 */
    W bcount;			/* 論理ブロックが物理ブロックより大きい場合に使用する。 */
    /* 物理ブロックを読みとるときの回数となる              */
    W cylinder;
    W head;
    W sector;
    W drive;			/* ドライブ番号 */
    UW partition;		/* パーティション番号 */
    W i, try;
    W done_length = 0;		/* 本当に読み込んだバイト数 */
    static B buff[BLOCK_SIZE * 2];
    UW bp;
    ER ret;
    ER error;
    UW bufstart;
    UW buflength;
    UW parstart;
    ER reply_error;

    drive = IDE_GET_DRIVE(req->dd);
    partition = IDE_GET_PARTITION(req->dd);
    if (partition == IDE_WHOLE_DISK) {
	parstart = 0;
    }
    if ((drive < 0) ||
	(partition > (IDE_MAX_PARTITION + ext_partition))) {
	dbg_printf("[IDE] write_ide Illegal driver or partition number [%d/%d]\n",
		   drive, partition);
	goto bad;
    } else {
	parstart = ide_partition[drive][partition - 1].start * BLOCK_SIZE;
    }

    bufstart = ROUNDDOWN(req->start, BLOCK_SIZE);
    buflength = ROUNDUP(req->start + req->size, BLOCK_SIZE);
#ifdef DEBUG
    dbg_printf("[IDE] write_ide partition = %d parstart = %d bufstart = %d length = %d\n",
	       partition, parstart, bufstart, buflength - bufstart);	/* */
#endif

    /* まず、該当するブロックの最初にディスクの中身を読み取る
     */
    error = E_OK;
    ret = E_OK;

    for (bp = 0; bp < (buflength - bufstart); bp += BLOCK_SIZE) {
#ifdef DEBUG
	dbg_printf("[IDE] write_ide bp = %d, length = %d\n", bp, BLOCK_SIZE);
#endif
	/* バイトオフセットから物理ブロック番号への変換 */
	blockno = ((UW) (bp + bufstart + parstart) / BLOCK_SIZE)
	    * (BLOCK_SIZE / IDE_BLOCK_SIZE);
	bcount = BLOCK_SIZE / IDE_BLOCK_SIZE;

#define H	(ide_spec[IDE_GET_DRIVE (req->dd)].n_head)
#define S	(ide_spec[IDE_GET_DRIVE (req->dd)].n_sector)
#define LEN	IDE_BLOCK_SIZE

#ifdef USE_LBA
	drive |= LBA_MODE_BIT;
#endif
	for (i = 0; i < bcount; i++) {
#ifndef USE_LBA
	    head = (blockno % (H * S)) / S;
	    cylinder = (blockno / (H * S));
	    sector = (blockno % S) + 1;
#else
	    sector = LBA_8(blockno);
	    cylinder = LBA_16(blockno);
	    head = LBA_4(blockno);
#endif

#ifdef DEBUG
	    dbg_printf("[IDE] write_ide parstart %d(H = %d, C = %d, S = %d)\n",
		       parstart, head, cylinder, sector);	/* */
#endif
	    for (try = 0; try < IDE_RETRY; try++) {
#ifdef DEBUG
		dbg_printf("[IDE] write_ide get_data (%d, %d, %d, %d, 0x%x)\n",
			   drive, head, cylinder, sector,
			   (void *) &(buff[bp + (i * LEN)]));
#endif

		ret =
		    get_data(drive, head, cylinder, sector,
			     (void *) &(buff[bp + (i * LEN)]),
			     LEN / IDE_BLOCK_SIZE);
		if (ret > 0)
		    break;
	    }
	    if (ret > 0) {
		done_length += ret;
		blockno++;
	    } else {
		error = E_SYS;
		goto bad;
	    }
	}
    }

    /* 読み取ったデータ上の必要な部分に、書き換えたいデータを上書きする */
    done_length = sizeof(buff) - req->start + bufstart;
    if (req->size <= done_length)
	done_length = req->size;
    else
	dbg_printf("[IDE] write_ide write buffer is too small\n");
    memcpy(&buff[req->start - bufstart], req->dt, done_length);

#ifdef DEBUG_1
    {
	int i;

	dbg_printf("[IDE] write_ide buff = 0x%x\n", buff);
	for (i = 0; i < 20 - 1; i++) {
	    dbg_printf("0x%x, ", buff[i]);
	}
	dbg_printf("0x%x\n", buff[i]);
    }
#endif

    /* 書き換えたデータを再度ディスク上に書き込む */
    done_length = 0;
    error = E_OK;
    ret = E_OK;

    for (bp = 0; bp < (buflength - bufstart); bp += BLOCK_SIZE) {
	/* バイトオフセットから物理ブロック番号への変換 */
	blockno = ((UW) (bp + bufstart + parstart) / BLOCK_SIZE)
	    * (BLOCK_SIZE / IDE_BLOCK_SIZE);
#if 0
	/* 計算済 */
	bcount = BLOCK_SIZE / IDE_BLOCK_SIZE;
#endif
#ifdef DEBUG
	dbg_printf("[IDE] write_ide bp = %d, no = %d\n", bp, blockno);
#endif

#define H	(ide_spec[IDE_GET_DRIVE (req->dd)].n_head)
#define S	(ide_spec[IDE_GET_DRIVE (req->dd)].n_sector)
#define LEN	IDE_BLOCK_SIZE

	for (i = 0; i < bcount; i++) {
#ifndef USE_LBA
	    head = (blockno % (H * S)) / S;
	    cylinder = (blockno / (H * S));
	    sector = (blockno % S) + 1;
#else
	    sector = LBA_8(blockno);
	    cylinder = LBA_16(blockno);
	    head = LBA_4(blockno);
#endif

#ifdef DEBUG
	    dbg_printf("[IDE] write_ide (H = %d, C = %d, S = %d)\n", head,
		       cylinder, sector);
#endif
	    for (try = 0; try < IDE_RETRY; try++) {
#ifdef DEBUG
		dbg_printf("[IDE] write_ide write_data (%d, %d, %d, %d, 0x%x)\n",
			   drive, head, cylinder, sector,
			   (void *) &(buff[bp + (i * LEN)]));
#endif
		ret =
		    put_data(drive, head, cylinder, sector,
			     (void *) &(buff[bp + (i * LEN)]),
			     LEN / IDE_BLOCK_SIZE);
		if (ret > 0)
		    break;
	    }
	    if (ret > 0) {
		done_length += ret;
		blockno++;
	    } else {
		error = E_SYS;
		goto bad;
	    }
	}
    }

#ifdef notdef
    done_length -= req->start - bufstart;
#endif
    done_length =
	((done_length < req->size) ? done_length : req->size);
    if (done_length > BLOCK_SIZE * 2) {
	dbg_printf("[IDE] write_ide buffer overflow (size = %d)\n", done_length);
	error = E_SYS;
	goto bad;
    }

    res->dd = req->dd;
    res->a_size = done_length;
    res->errcd = E_OK;
    res->errinfo = 0;
    reply_error = rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    if (reply_error) {
	dbg_printf("[IDE] write_ide rpl_rdv error = %d\n", reply_error);
    }
    return (E_OK);

  bad:
    res->dd = req->dd;
    res->a_size = done_length;
    res->errcd = error;
    res->errinfo = error;
    reply_error = rpl_rdv(rdvno, packet, sizeof(DDEV_RES));
    if (reply_error) {
	dbg_printf("[IDE] write_ide rpl_rdv error = %d\n", reply_error);
    }
    return (error);

#undef H
#undef S
#undef LEN
}

/************************************************************************
 *	control_ide
 */
W control_ide(RDVNO rdvno, devmsg_t * packet)
{
    DDEV_CTL_REQ * req = &(packet->req.body.ctl_req);
    DDEV_CTL_RES * res = &(packet->res.body.ctl_res);
    ER error = E_OK;
    W drive;
    struct ide_partition *p;
    W i;

    drive = IDE_GET_DRIVE(req->dd);
    switch (req->cmd) {
    case IDE_GET_STAT:
	/* IDE の情報を取り出す */
	error = read_stat(drive, &ide_spec[drive]);
	break;

    case IDE_GET_GEOMETRIC:
	dbg_printf("[IDE] control_ide IDE_GET_GEOMETRIC: start. (drive = %d)\n", drive);
	error = read_partition(drive);
	p = (struct ide_partition *) res->res;
	for (i = 0; i < IDE_MAX_PARTITION; i++) {
	    p[i].boot_flag = ide_partition[drive][i].boot_flag;
#ifdef notdef
	    p[i].start_head = ide_partition[drive][i].start_head;
	    p[i].start_sector = ide_partition[drive][i].start_sector;
	    p[i].start_cylinder = ide_partition[drive][i].start_cylinder;
#endif
	    p[i].length = ide_partition[drive][i].length;
	    p[i].type = ide_partition[drive][i].type;
	    p[i].start = ide_partition[drive][i].start;
	}
	break;


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


ER read_partition(W drive)
{
    static UB buf[BLOCK_SIZE];
    W rlength;
    W i;
    ER error = E_OK;
    int cylinder, sector, head;
    unsigned int true_block;
    int have_ext_partition;
    struct ide_partition pt_buf[IDE_MAX_PARTITION], *tp;
    unsigned int ext_offset;
#ifdef notdef
    struct ide_partition *table;
#endif

    /* drive 0 に対しては初期化の際に実行される */
    error = read_stat(drive, &ide_spec[drive]);
    if (error) {
	return (error);
    }

    for (i = 0; i < IDE_RETRY; ++i) {
	rlength = get_data(drive, 0, 0, 1, (B*)buf, 1);	/* H = 0, C = 0, S = 1 */
	if (rlength > 0)
	    break;
    }
    if (rlength == 0) {
	dbg_printf("[IDE] read_partition can not read partition table of drive %d\n",
		   drive);
	return (E_SYS);
    }
#ifdef notdef
    dbg_printf("[IDE] read_partition get_data length = %d\n", rlength);
#endif
    memcpy(ide_partition[drive], &buf[PARTITION_OFFSET],
	  sizeof(struct ide_partition) * IDE_MAX_PARTITION);

    ext_partition = 0;
    have_ext_partition = -1;
    for (i = 0; i < IDE_MAX_PARTITION; i++) {
#ifdef notdef
	dbg_printf("[IDE] read_partition [%d]: start = %d, length = %d\n",
		   i,
		   ide_partition[drive][i].start,
		   ide_partition[drive][i].length);
#endif
	/* 拡張パーティションの判定 */
	switch (ide_partition[drive][i].type) {
	case 0x05:
	case 0x0F:
	case 0x85:
	    have_ext_partition = i;
	    break;
	}
    }

    if (have_ext_partition >= 0) {
	ext_offset = ide_partition[drive][have_ext_partition].start;
	ide_partition[drive][IDE_MAX_PARTITION].start = ext_offset;
	do {
	    true_block = ide_partition[drive][have_ext_partition].start;
	    sector = LBA_8(true_block);
	    cylinder = LBA_16(true_block);
	    head = LBA_4(true_block);
	    rlength =
		get_data(drive | LBA_MODE_BIT, head, cylinder, sector, (B*)buf,
			 1);
	    memcpy((char *) &pt_buf, &buf[PARTITION_OFFSET],
		  sizeof(struct ide_partition) * IDE_MAX_PARTITION);
#ifdef notdef
	    dbg_printf
		("[IDE] read_partition partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
		 IDE_MAX_PARTITION + ext_partition, pt_buf[0].type,
		 pt_buf[0].start, pt_buf[0].length, pt_buf[0].boot_flag);
#endif
	    tp = &ide_partition[drive][IDE_MAX_PARTITION + ext_partition];
	    tp->type = pt_buf[0].type;
	    tp->start += pt_buf[0].start;
	    tp->length = pt_buf[0].length;
	    tp->boot_flag = pt_buf[0].boot_flag;
	    ++ext_partition;
	    if (ext_partition < IDE_MAX_EXT_PARTITION) {
		++tp;
		tp->start = pt_buf[1].start + ext_offset;
	    }
	    have_ext_partition = IDE_MAX_PARTITION + ext_partition;
	} while (pt_buf[1].type && ext_partition < IDE_MAX_EXT_PARTITION);
    }
#ifdef notdef
    for (i = 0, tp = ide_partition[drive];
	 i < IDE_MAX_PARTITION + ext_partition; ++i, ++tp) {
	dbg_printf
	    ("[IDE] read_partition partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
	     i, tp->type, tp->start, tp->length, tp->boot_flag);
    }
#endif

#ifdef notdef
    /* 以下のルーチンは obsolute */
    if (rlength > 0) {
	table = ide_partition[drive];
	for (i = 0; i < IDE_MAX_PARTITION; i++) {
	    table[i].boot_flag =
		buf[
		    (PARTITION_OFFSET + BOOT_FLAG_OFFSET +
		     (PARTITION_SIZE * i))];
#ifdef notdef
	    table[i].start_head =
		buf[
		    (PARTITION_OFFSET + HEAD_OFFSET +
		     (PARTITION_SIZE * i))];
	    table[i].start_sector =
		buf[
		    (PARTITION_OFFSET + SECTOR_OFFSET +
		     (PARTITION_SIZE * i))];
	    table[i].start_cylinder =
		buf[
		    (PARTITION_OFFSET + CYLINDER_OFFSET +
		     (PARTITION_SIZE * i))];
#endif

	    (char *) ip =
		&buf[
		     (PARTITION_OFFSET + TOTAL_SECTOR_OFFSET +
		      (PARTITION_SIZE * i))];
	    table[i].length = *ip;
	    table[i].type =
		buf[
		    (PARTITION_OFFSET + TYPE_OFFSET +
		     (PARTITION_SIZE * i))];
#ifdef notdef
	    table[i].start = (table[i].start_sector - 1)
		+ (table[i].start_head * ide_spec[drive].n_sector)
		+ (table[i].start_cylinder
		   * ide_spec[drive].n_head * ide_spec[drive].n_sector);
#else
	    table[i].start = (table[i].start_sector - 1)
		+ (table[i].start_head * ide_spec[drive].now_sector_track)
		+ (table[i].start_cylinder
		   * ide_spec[drive].now_head
		   * ide_spec[drive].now_sector_track);
#endif
	    dbg_printf
		("[IDE] read_partition ide partition: drive = %d, partition = %d, start = %d, size = %d\n",
		 drive, i, table[i].start, table[i].length);
	}
	return (E_OK);
    }
#endif				/* notdef */

    initialized = 1;
    return (E_OK);
}


void intr_ide(void)
{
    dis_int();
    ide_intr_flag = TRUE;
    pic_reset_mask(ir_ide_primary);
    ena_int();
}


void busywait(W x)
{
    int i, j;
    int tmp;

    for (i = 0; i < x; i++)
	for (j = 0; j < 100; j++)
	    tmp = j;
}
