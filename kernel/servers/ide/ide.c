/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/ide.c,v 1.15 2000/01/22 10:59:30 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/ide.c,v 1.15 2000/01/22 10:59:30 naniwa Exp $";

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
#include "ide.h"
#include "../../../include/mpu/io.h"


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
static W mydevid;		/* 自分自身のid */
static ID recvport;		/* 要求受けつけ用ポート */
static W initialized;

/*
 *	局所関数群の定義
 */
static void main_loop(void);
static int init_ide_driver(void);
static void doit(DDEV_REQ * packet);

/*
 * FD デバイスドライバの main 関数
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
    if (init_ide_driver() != E_OK) {
	slp_tsk();
    }

    /*
     * 立ち上げメッセージ
     */
    dbg_printf("IDE disk driver started. receive port is %d\n", recvport);

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
/*      printf ("ide: get_req\n");	/* */
	get_req(recvport, &req, &rsize);
	switch (sys_errno) {
	case E_OK:
	    /* 正常ケース */
#ifdef DEBUG
	    printf("ide: receive packet type = %d\n", req.header.msgtyp);
#endif
	    doit(&req);
	    break;

	default:
	    /* Unknown error */
	    dbg_printf("ide: get_req() Unknown error(error = %d)\n",
		       sys_errno);
	    dbg_printf("IDE driver is halt.\n");
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

    if (!initialized) {
	dbg_printf("[IDE] devie is not initialized\n");
    }
    switch (packet->header.msgtyp) {
    case DEV_OPN:
	/* デバイスのオープン */
	open_ide(packet->header.mbfid, &(packet->body.opn_req));
	break;

    case DEV_CLS:
	/* デバイスのクローズ */
	close_ide(packet->header.mbfid, &(packet->body.cls_req));
	break;

    case DEV_REA:
	read_ide(packet->header.mbfid, &(packet->body.rea_req));
	break;

    case DEV_WRI:
	write_ide(packet->header.mbfid, &(packet->body.wri_req));
	break;

    case DEV_CTL:
	control_ide(packet->header.mbfid, &(packet->body.ctl_req));
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
    int i;
    ID root_dev;
    ER error;

    initialized = 0;
    if (init_ide() != E_OK) {
	return E_SYS;
    }

    /*
     * 要求受けつけ用のポートを初期化する。
     */
#ifdef notdef
    recvport = get_port(sizeof(DDEV_RES), sizeof(DDEV_RES));
#else
    recvport = get_port(0, sizeof(DDEV_RES));
#endif
    if (recvport <= 0) {
	dbg_printf("IDE: cannot make receive porrt.\n");
	slp_tsk();
	/* メッセージバッファ生成に失敗 */
    }

    error = regist_port(IDE_DRIVER, recvport);
    if (error != E_OK) {
	/* error */
    }

    init_log();
    return E_OK;
}

/*
 * init_ide --- IDE ドライバの初期化
 *
 */
W init_ide(void)
{
    ER err;
    int status;
#ifdef notdef
    T_DINT pkt;
#endif

#ifdef notdef
    /* 割込みは使用しない */
    pkt.intatr = ATR_INTR;
    pkt.inthdr = (FP) intr_ide;
    err = def_int(INT_IDE0, &pkt);
    if (err != E_OK) {
	dbg_printf("ide: error on def_int (errno = %d)\n", err);
	return (err);
    }
    reset_intr_mask(IDE0_INTR_MASK);
#endif

    outb(IDE_CONTROL_REG, IDE_SRST);	/* ソフトウェア・リセット */
    outb(IDE_CONTROL_REG, IDE_nIEN);	/* 割込み禁止 */
    status = ide_wait_while_busy();
    dbg_printf("IDE init status = 0x%x/0x%x\n",
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
W open_ide(ID caller, DDEV_OPN_REQ * packet)
{
    DDEV_RES res;
    W drive;			/* ドライブ番号 */
    UW partition;		/* パーティション番号 */
    int i;

#ifdef notdef
    outb(0x439, (inb(0x439) & 0xfb));	/* DMA Accsess Control over 1MB */
    outb(0x29, (0x0c | 0));	/* Bank Mode Reg. 16M mode */
    outb(0x29, (0x0c | 1));	/* Bank Mode Reg. 16M mode */
    outb(0x29, (0x0c | 2));	/* Bank Mode Reg. 16M mode */
    outb(0x29, (0x0c | 3));	/* Bank Mode Reg. 16M mode */
#endif
    res.body.opn_res.dd = packet->dd;
    drive = IDE_GET_DRIVE(packet->dd);
    partition = IDE_GET_PARTITION(packet->dd);
    if ((partition > IDE_WHOLE_DISK) &&
	(partition <= (IDE_MAX_PARTITION + ext_partition))) {
	res.body.opn_res.size =
	    ide_partition[drive][partition - 1].length * IDE_BLOCK_SIZE;
    } else if (partition == IDE_WHOLE_DISK) {
	res.body.opn_res.size = ((UW) ide_spec[drive].now_sector) *
	    ((UW) ide_spec[drive].now_head) *
	    ((UW) ide_spec[drive].now_cylinder);
	if (res.body.opn_res.size >= IDE_USABLE_SIZE) {
	    res.body.opn_res.size = MAX_UINT;
	} else {
	    res.body.opn_res.size *= IDE_BLOCK_SIZE;
	}
    } else {
	res.body.opn_res.size = 0;
    }
#if 0
    dbg_printf("partition #%d size = %d\n", partition,
	       res.body.opn_res.size);
#endif
    res.body.opn_res.errcd = E_OK;
    res.body.opn_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
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
W close_ide(ID caller, DDEV_CLS_REQ * packet)
{
    DDEV_RES res;

    res.body.cls_res.dd = packet->dd;
    res.body.cls_res.errcd = E_OK;
    res.body.cls_res.errinfo = E_OK;
    snd_mbf(caller, sizeof(res), &res);
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
W read_ide(ID caller, DDEV_REA_REQ * packet)
{
    DDEV_RES res;
    W blockno;			/* 物理ブロック番号 */
    W bcount;			/* 論理ブロックが物理ブロックより大きい場合に使用する。 */
    /* 物理ブロックを読みとるときの回数となる              */
    W cylinder;
    W head;
    W sector;
    W drive;			/* ドライブ番号 */
    UW partition;		/* パーティション番号 */
    W i, try;
    W done_length;		/* 本当に読み込んだバイト数 */
    static B buff[BLOCK_SIZE * 2];
    UW bp;
    ER ret;
    ER error;
    UW bufstart;
    UW buflength;
    UW parstart;

    drive = IDE_GET_DRIVE(packet->dd);
    partition = IDE_GET_PARTITION(packet->dd);
    if (partition == IDE_WHOLE_DISK) {
	parstart = 0;
    }
    if ((drive < 0) ||
	(partition > (IDE_MAX_PARTITION + ext_partition))) {
	dbg_printf("[IDE] Illegal driver or partition number [%d/%d]\n",
		   drive, partition);
	goto bad;
    } else {
	parstart = ide_partition[drive][partition - 1].start * BLOCK_SIZE;
    }

    bufstart = ROUNDDOWN(packet->start, BLOCK_SIZE);
    buflength = ROUNDUP(packet->start + packet->size, BLOCK_SIZE);
#ifdef DEBUG
    dbg_printf("partition = %d, bufstart = %d, buflength = %d\n",
	       partition, bufstart, buflength);	/* */
#endif

    done_length = 0;
    error = E_OK;
    ret = E_OK;

    for (bp = 0; bp < (buflength - bufstart); bp += BLOCK_SIZE) {
#ifdef DEBUG
	dbg_printf("read_ide: bp = %d, length = %d\n", bp, BLOCK_SIZE);	/* */
#endif
	/* バイトオフセットから物理ブロック番号への変換 */
	blockno = ((UW) (bp + bufstart + parstart) / BLOCK_SIZE)
	    * (BLOCK_SIZE / IDE_BLOCK_SIZE);
	bcount = BLOCK_SIZE / IDE_BLOCK_SIZE;

#define H	(ide_spec[IDE_GET_DRIVE (packet->dd)].n_head)
#define S	(ide_spec[IDE_GET_DRIVE (packet->dd)].n_sector)
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
	    dbg_printf("read_ide: (H = %d, C = %d, S = %d)\n", head,
		       cylinder, sector);	/* */
#endif
	    for (try = 0; try < IDE_RETRY; try++) {
#ifdef DEBUG
		dbg_printf("get_data (%d, %d, %d, %d, 0x%x)\n",
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
    dbg_printf("bcopy(): 0x%x, 0x%x, %d\n",
	       diff,
	       res.body.rea_res.dt,
	       (done_length < packet->size) ? done_length : packet->size);
#endif

    done_length =
	((done_length < packet->size) ? done_length : packet->size);
#ifdef notdef
    done_length -= packet->start - bufstart;
#endif
    if (done_length > BLOCK_SIZE * 2) {
	dbg_printf("buffer overflow (size = %d)\n",
		   (done_length <
		    packet->size) ? done_length : packet->size);
	error = E_SYS;
	goto bad;
    }

    bcopy(&buff[packet->start - bufstart],
	  res.body.rea_res.dt, done_length);
    res.body.rea_res.dd = packet->dd;
    res.body.rea_res.a_size = done_length;
    res.body.rea_res.errcd = E_OK;
    res.body.rea_res.errinfo = 0;
    snd_mbf(caller, sizeof(res), &res);
#ifdef DEBUG_1
    dbg_printf("ide: snd_mbuf done.\n");
#endif
    return (E_OK);

  bad:
    res.body.rea_res.dd = packet->dd;
    res.body.rea_res.a_size = done_length;
    res.body.rea_res.errcd = error;
    res.body.rea_res.errinfo = error;
    snd_mbf(caller, sizeof(res), &res);
    return (error);

#undef H
#undef S
#undef LEN
}


/************************************************************************
 *	write_ide
 */
W write_ide(ID caller, DDEV_WRI_REQ * packet)
{
    DDEV_RES res;
    W blockno;			/* 物理ブロック番号 */
    W bcount;			/* 論理ブロックが物理ブロックより大きい場合に使用する。 */
    /* 物理ブロックを読みとるときの回数となる              */
    W cylinder;
    W head;
    W sector;
    W drive;			/* ドライブ番号 */
    UW partition;		/* パーティション番号 */
    W i, try;
    W done_length;		/* 本当に読み込んだバイト数 */
    static B buff[BLOCK_SIZE * 2];
    UW bp;
    ER ret;
    ER error;
    UW bufstart;
    UW buflength;
    UW parstart;

    drive = IDE_GET_DRIVE(packet->dd);
    partition = IDE_GET_PARTITION(packet->dd);
    if (partition == IDE_WHOLE_DISK) {
	parstart = 0;
    }
    if ((drive < 0) ||
	(partition > (IDE_MAX_PARTITION + ext_partition))) {
	dbg_printf("[IDE] Illegal driver or partition number [%d/%d]\n",
		   drive, partition);
	goto bad;
    } else {
	parstart = ide_partition[drive][partition - 1].start * BLOCK_SIZE;
    }

    bufstart = ROUNDDOWN(packet->start, BLOCK_SIZE);
    buflength = ROUNDUP(packet->start + packet->size, BLOCK_SIZE);
#ifdef DEBUG
    dbg_printf("partition = %d parstart = %d bufstart = %d length = %d\n",
	       partition, parstart, bufstart, buflength - bufstart);	/* */
#endif

    /* まず、該当するブロックの最初にディスクの中身を読み取る
     */
    done_length = 0;
    error = E_OK;
    ret = E_OK;

    for (bp = 0; bp < (buflength - bufstart); bp += BLOCK_SIZE) {
#ifdef DEBUG
	dbg_printf("write_ide: bp = %d, length = %d\n", bp, BLOCK_SIZE);
#endif
	/* バイトオフセットから物理ブロック番号への変換 */
	blockno = ((UW) (bp + bufstart + parstart) / BLOCK_SIZE)
	    * (BLOCK_SIZE / IDE_BLOCK_SIZE);
	bcount = BLOCK_SIZE / IDE_BLOCK_SIZE;

#define H	(ide_spec[IDE_GET_DRIVE (packet->dd)].n_head)
#define S	(ide_spec[IDE_GET_DRIVE (packet->dd)].n_sector)
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
	    dbg_printf("write_ide: parstart %d(H = %d, C = %d, S = %d)\n",
		       parstart, head, cylinder, sector);	/* */
#endif
	    for (try = 0; try < IDE_RETRY; try++) {
#ifdef DEBUG
		dbg_printf("get_data (%d, %d, %d, %d, 0x%x)\n",
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
    done_length = sizeof(buff) - packet->start + bufstart;
    if (packet->size <= done_length)
	done_length = packet->size;
    else
	dbg_printf("[IDE] write buffer is too small\n");
    bcopy(packet->dt, &buff[packet->start - bufstart], done_length);

#ifdef DEBUG_1
    {
	int i;

	dbg_printf("buff = 0x%x\n", buff);
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
	dbg_printf("write_ide: bp = %d, no = %d\n", bp, blockno);
#endif

#define H	(ide_spec[IDE_GET_DRIVE (packet->dd)].n_head)
#define S	(ide_spec[IDE_GET_DRIVE (packet->dd)].n_sector)
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
	    dbg_printf("write_ide: (H = %d, C = %d, S = %d)\n", head,
		       cylinder, sector);
#endif
	    for (try = 0; try < IDE_RETRY; try++) {
#ifdef DEBUG
		dbg_printf("write_data (%d, %d, %d, %d, 0x%x)\n",
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
    done_length -= packet->start - bufstart;
#endif
    done_length =
	((done_length < packet->size) ? done_length : packet->size);
    if (done_length > BLOCK_SIZE * 2) {
	dbg_printf("buffer overflow (size = %d)\n", done_length);
	error = E_SYS;
	goto bad;
    }

    res.body.wri_res.dd = packet->dd;
    res.body.wri_res.a_size = done_length;
    res.body.wri_res.errcd = E_OK;
    res.body.wri_res.errinfo = 0;
    snd_mbf(caller, sizeof(res), &res);
#ifdef DEBUG_1
    dbg_printf("ide: snd_mbuf done.\n");
#endif
    return (E_OK);

  bad:
    res.body.wri_res.dd = packet->dd;
    res.body.wri_res.a_size = done_length;
    res.body.wri_res.errcd = error;
    res.body.wri_res.errinfo = error;
    snd_mbf(caller, sizeof(res), &res);
    return (error);

#undef H
#undef S
#undef LEN
}

/************************************************************************
 *	control_ide
 */
W control_ide(ID caller, DDEV_CTL_REQ * packet)
{
    DDEV_RES res;
    ER error = E_OK;
    W drive;
    struct ide_partition *p;
    W i;

    drive = IDE_GET_DRIVE(packet->dd);
    switch (packet->cmd) {
    case IDE_GET_STAT:
	/* IDE の情報を取り出す */
	error = read_stat(drive, &ide_spec[drive]);
	break;

    case IDE_GET_GEOMETRIC:
	dbg_printf("IDE_GET_GEOMETRIC: start. (drive = %d)\n", drive);
	error = read_partition(drive);
	p = (struct ide_partition *) res.body.ctl_res.res;
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
    res.body.ctl_res.dd = packet->dd;
    res.body.ctl_res.errcd = error;
    res.body.ctl_res.errinfo = error;
    snd_mbf(caller, sizeof(res), &res);
    return (error);
}


ER read_partition(W drive)
{
    static UB buf[BLOCK_SIZE];
    W rlength;
    UW *ip;
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
	rlength = get_data(drive, 0, 0, 1, buf, 1);	/* H = 0, C = 0, S = 1 */
	if (rlength > 0)
	    break;
    }
    if (rlength == 0) {
	dbg_printf("[IDE] can not read partition table of drive %d\n",
		   drive);
	return (E_SYS);
    }
#ifdef notdef
    dbg_printf("IDE: get_data length = %d\n", rlength);
#endif
    bcopy(&buf[PARTITION_OFFSET], ide_partition[drive],
	  sizeof(struct ide_partition) * IDE_MAX_PARTITION);

    ext_partition = 0;
    have_ext_partition = -1;
    for (i = 0; i < IDE_MAX_PARTITION; i++) {
#ifdef notdef
	dbg_printf("[%d]: start = %d, length = %d\n",
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
		get_data(drive | LBA_MODE_BIT, head, cylinder, sector, buf,
			 1);
	    bcopy(&buf[PARTITION_OFFSET], (char *) &pt_buf,
		  sizeof(struct ide_partition) * IDE_MAX_PARTITION);
#ifdef notdef
	    dbg_printf
		("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
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
	    ("partition[%d] type = 0x%x, start = %d, length = %d, bootable = %d\n",
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
		("ide partition: drive = %d, partition = %d, start = %d, size = %d\n",
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
    reset_intr_mask(IDE0_INTR_MASK);
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
