/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/ide/ide.h,v 1.8 1999/04/18 18:43:27 monaka Exp $ */

/*
 *
 * 
 *
 *
 */

#ifndef __IDE_H__
#define __IDE_H__	1


#include <itron.h>
#include <types.h>
#include <errno.h>
#include "../../ITRON/ibmpc/ibmpc.h"
#include "../../lib/libkernel/dma.h"
#include "../../lib/libkernel/device.h"
#include "../console/console.h"
#include "../../servers/port-manager.h"

#define USE_LBA

#define IDE_BLOCK_SIZE		512
#define IDE_MAX_PARTITION	4
#define IDE_MAX_EXT_PARTITION	5	/* 拡張パーティションの最大数 */
#define IDE_WHOLE_DISK		0
#define MAX_UINT		(0xFFFFFFFFUL)
#define IDE_USABLE_SIZE		(MAX_UINT/IDE_BLOCK_SIZE)

#define PARTITION_SIZE		(0x10)
#define PARTITION_OFFSET	(0x1be)
#define BOOT_FLAG_OFFSET	(0)
#define HEAD_OFFSET		(1)
#define SECTOR_OFFSET		(2)
#define CYLINDER_OFFSET		(3)
#define TOTAL_SECTOR_OFFSET	(12)
#define TYPE_OFFSET		(4)


#define ROUNDDOWN(x,b)	((x / b) * b)
#ifndef ROUNDUP
#define ROUNDUP(x,b)	((x + (b -1)) / b * b)
#endif

#define MAXIDE		2

#define DRIVE_SHIFT	4
#define ATAPI_BITS	0xA0
#define CYLINDER_SHIFT	8

#define IDE_DRIVER	"driver.ide"

#define BLOCK_SIZE	512

#define IDE_INT_VECTOR	38
#define IDE_MASK_LEVEL	0

#define IDE_RETRY	10


/*
 * デバイス番号指定
 *
 * IDE のデバイス番号は、次の形式で管理される。
 *
 *
 * MSB から 16 bit ...... 未使用
 *           8 bit ...... ドライブ番号
 *           8 bit ...... パーティション番号 (0 が全体。1 がパーティション1を指す
 *
 */
#define IDE_GET_DRIVE(dd)	((dd & 0xff00) >> 8)
#define IDE_GET_PARTITION(dd)	(dd & 0xff)

#define LBA_8(x)	((x) & 0xFF)
#define LBA_16(x)	(((x) >> 8) & 0xFFFFUL)
#define LBA_4(x)	(((x) >> 24) & 0x0F)
#define LBA_MODE_BIT	4

/*********************************************************************
 *
 *	IDEC 制御用定数の定義
 *
 */
/*
 * ポートアドレス
 */

/*
 * ドライバ ステータス定数
 *	これらの定数は、fd_data[] の status メンバに入る。
 *
 */
#define DRIVE_FREE		0x00
#define DRIVE_USING		0x80
#define DRIVE_BUSY		0x01
#define DRIVE_WAIT_INT		0x02
#define DRIVE_DMA		0x04
#define DRIVE_NOINIT		0x08
#define ON_MOTOR		1
#define OFF_MOTOR		0

#define INT_IDE0		46	/* Primary IDE HD */
#define INT_IDE1		47	/* Secondary IDE HD */

#define IDE0_INTR_MASK		14
#define IDE1_INTR_MASK		15

#define IDE_STAT_REG		0x1f7
#define IDE_COMMAND_REG		0x1f7
#define IDE_ALT_STAT_REG	0x3f6
#define IDE_CONTROL_REG		0x3f6
#define IDE_ADDR_REG		0x3f7	/* ディジタルインプットレジスタ */
#define IDE_ERROR_REG		0x1f1
#define IDE_DATA_REG		0x1f0
#define IDE_FEATURE_REG		0x1f1
#define IDE_SCOUNT_REG		0x1f2
#define IDE_SNUMBER_REG		0x1f3
#define IDE_CYL_LOW_REG		0x1f4
#define IDE_CYL_HIGH_REG	0x1f5
#define IDE_DRIVE_REG		0x1f6	/* ドライブ/ヘッドレジスタ */

#define	IDE_CMD_READ_INFO	0xEC
#define IDE_CMD_READ		0x20	/* リトライあり */
#define IDE_CMD_WRITE		0x30	/* リトライあり */
#define IDE_CMD_VERIFY		0x40
#define IDE_CMD_FORMAT		0x50
#define IDE_CMD_SEEK		0x70
#define IDE_CMD_DIAGNOSE	0x90
#define IDE_CMD_SPECIFY		0x91
#define IDE_CMD_RECALIBRATE	0x10
#define IDE_CMD_SLEEP		0xE6
#define IDE_CMD_STANDBY		0xE2
#define IDE_CMD_STANDBYI	0xE0

/* ステータス・レジスタのフラグ */
#define IDE_DRQ			0x08
#define IDE_DRDY		0x40
#define IDE_BSY			0x80
#define IDE_ERR			0x01

/* デバイス・コントロール・レジスタに書き込む命令 */
#define IDE_nIEN		0x02 /* 割込み禁止 */
#define IDE_SRST		0x04 /* ソフトウェアリセット */

/* Control command */
#define IDE_GET_STAT		0x01
#define IDE_GET_GEOMETRIC	0x02

struct ide_id
{
  unsigned short	config;				/*  0 */
  unsigned short	n_cylinder;			/*  1 */
  unsigned short	pad0;				/*  2 */
  unsigned short	n_head;				/*  3 */
  unsigned short	track_byte_unformat;		/*  4 */
  unsigned short	sector_byte_unformat;		/*  5 */
  unsigned short	n_sector;			/*  6 */
  unsigned short	pad1[3];			/*  7,8,9 */
  unsigned char		number[20];			/* 10 - 19 */
  unsigned short	buffer_type;			/* 20 */
  unsigned short	buffer_size;			/* 21 */
  unsigned short	pad2;				/* 22 */
  unsigned char		firm_ware[8];			/* 23 - 26 */
  unsigned char		model[40];			/* 27 - 46 */
  unsigned short	pad3;				/* 47 */
  unsigned short	double_word_io;			/* 48 */
  unsigned short	dma_support;			/* 49 */
  unsigned short	pad4;				/* 50 */
  unsigned short	pio_config;			/* 51 */
  unsigned short	dma_config;			/* 52 */
  unsigned short	trans_mode;			/* 53 */
  unsigned short	now_cylinder;			/* 54 */
  unsigned short	now_head;			/* 55 */
  unsigned short	now_sector;			/* 56 */
  unsigned short	now_all_sector[2];		/* 57-58 */
  unsigned short	pad5[197];
};

struct spec
{
  B	*typename;	/* タイプ名; */
  UW	head;		/* ヘッドの数 */
  UW	cylinder;	/* シリンダ数 */
  UW	sector;		/* セクタ数 / トラック */
  UW	length;		/* セクタ長*/
  UW	dtl;		/* セクタ長 (lencode = 0 の時に使用)。使用しない場合には 0xFF */
  UW	retry;		/* 最大リトライ数 */	
  UW	powersave;	/* パワーセーブモード */
};


struct ide_partition
{
#ifdef notdef
  int	boot_flag;
  int	start_head;
  int	start_sector;
  int	start_cylinder;
  int	start_block;
  int	total_block;
  int	type;
#else
  unsigned char		boot_flag;
  unsigned char		begin[3];	/* CHS: 使わない */
  unsigned char		type;
  unsigned char		end[3];		/* CHS: 使わない */
  unsigned long		start;
  unsigned long		length;
#endif
};


/* ide.c */
extern W    	init_ide(void);	/* 初期化		*/
extern W    	open_ide();	/* オープン		*/
extern W  	close_ide();	/* クローズ		*/
extern W    	read_ide();	/* 読み込み		*/
extern W    	write_ide();	/* 書き込み		*/
extern W    	control_ide();	/* コントロール		*/
extern W    	status_ide();	/* ステータス		*/
extern W	ide_intr_flag;
extern ER	read_partition (W drive);

extern void	intr_ide ();
extern void	busywait (W);


/* idelow.c */
extern ER	put_data (W drive, UW head, UW cyl, UW sec, B *buf, W length);
extern ER	get_data (W drive, UW head, UW cyl, UW sec, B *buf, W length);
extern int      ide_wait_while_busy();
extern int 	ide_wait_device_ready();
extern void 	ide_send_command(int cmd);
extern void	ide_recalibrate(int drive);

#endif /* __IDE_H__ */


