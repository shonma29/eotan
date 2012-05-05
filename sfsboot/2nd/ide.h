#ifndef __IDE_H__
#define __IDE_H__

#define USE_LBA

#define IDE_BLOCK_SIZE		512
#define IDE_MAX_PARTITION	4
#define IDE_MAX_EXT_PARTITION	5	/* 拡張パーティションの最大数 */

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

#define	IDE_CMD_READ_INFO	0xec
#define IDE_CMD_READ		0x20

#define IDE_DRQ			0x08

#define HEAD(table,part)	(table[0x1bf])
#define CYLINDER(table,part)	(table[0x1c1])
#define SECTOR(table,part)	(table[0x1c0])

#define PARTITION_SIZE		(0x10)
#define PARTITION_OFFSET	(0x1be)
#define BOOT_FLAG_OFFSET	(0)
#define HEAD_OFFSET		(1)
#define SECTOR_OFFSET		(2)
#define CYLINDER_OFFSET		(3)
#define TOTAL_SECTOR_OFFSET	(12)
#define TYPE_OFFSET		(4)

#define LBA_8(x)	((x) & 0xFF)
#define LBA_16(x)	(((x) >> 8) & 0xFFFFUL)
#define LBA_4(x)	(((x) >> 24) & 0x0F)
#define LBA_MODE_BIT	4

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
  unsigned short	ecc_bytes;			/* 22 - ecc bytes */
  unsigned char		firm_ware[8];			/* 23 - 26 */
  unsigned char		model[40];			/* 27 - 46 */
  unsigned short	pad2;
  unsigned short	double_word_io;
  unsigned short	dma_support;
  unsigned short	pad3;
  unsigned short	pio_config;
  unsigned short	dma_config;
  unsigned short	trans_mode;
  unsigned short	now_cylinder;
  unsigned short	now_head;
  unsigned short	now_sector_track;
  unsigned short	now_sector[2];
/*  unsigned short	pad4[197]; */
};


struct ide_partition
{
  unsigned char		boot_flag;
  unsigned char		begin[3];	/* header, sector, cylinder
					 * の順にならんでいる。
					 * ただし、sector の上位 2 ビットは、
					 * シリンダの上位 2 ビットを補完する。
					 */
  unsigned char		type;
  unsigned char		end[3];		/* header, sector, cylinder
					 * の順にならんでいる。
					 * ただし、sector の上位 2 ビットは、
					 * シリンダの上位 2 ビットを補完する。
					 */
  unsigned long		start;
  unsigned long		length;
};


extern int ide_init (void);
extern int ide_read (int drive, int partition, ULONG blockno, BYTE *buf, int length);
extern int test_ide (char **arg);
extern void ide_read_dump (char **arg);
extern int get_ide_parameter (struct ide_id *id);
extern void ide_id (void);

#endif /* __IDE_H__ */
