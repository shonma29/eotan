/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* sfs_fs.h - SFS のための定義
 *
 *
 * $Log: sfs_fs.h,v $
 * Revision 1.11  1999/05/28 15:48:37  naniwa
 * sfs ver 1.1
 *
 * Revision 1.10  1997/07/03 14:24:36  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.9  1996/11/18  13:46:11  night
 * ファイルの中身を作成。
 *
 * Revision 1.8  1996/11/16  12:54:47  night
 * SFS_FMT_REG、SFS_FMT_DIR、SFS_FMT_DEV マクロの値を変更した。
 *
 * Revision 1.7  1996/11/14  13:18:33  night
 * 二重間接ブロックのための定義を追加。
 *
 * Revision 1.6  1996/11/13  12:54:00  night
 * sfs_inode 構造体の形式の変更。
 *
 * Revision 1.5  1996/11/12  11:32:25  night
 * SFS_DIRECT_BLOCK_ENTRY マクロの値を 100 から 53 に変更した。
 * (sfs_inode 構造体の大きさを 256 バイトに合わせるため)
 *
 * Revision 1.4  1996/11/10  11:56:20  night
 * sfs_inode 構造体にブロック管理用の領域を追加。
 * (sfs_i_direct 要素)
 *
 * Revision 1.3  1996/11/08  11:06:18  night
 * SFS ファイルシステムの各種構造体を定義した。
 *
 * Revision 1.2  1996/11/07  21:12:15  night
 * 文字コードを EUC に変更した。
 *
 * Revision 1.1  1996/11/07  12:50:56  night
 * 最初の登録
 *
 */


#ifndef __FS_SFS_H__
#define __FS_SFS_H__	1

#include "../core/types.h"


#define SFS_MAGIC			0x12345678
#define SFS_MAXNAMELEN			14
#define SFS_VERSION_HI			2
#define SFS_VERSION_LO			1
#define SFS_BLOCK_SIZE 512

#define SFS_INDIRECT_BLOCK_ENTRY	(112)

#define SFS_INDIRECT_BLOCK		(SFS_BLOCK_SIZE / sizeof(UW))

struct sfs_superblock
{
  UW	magic;
  H	version_hi;
  H	version_lo;
  H	blksize;		/* ブロックサイズ */

  UW	nblock;
  UW	freeblock;

  UW	bitmapsize;

  UW	ninode;
  UW	freeinode;

  UW	isearch;		/* この番号以下の inode は使用中 */
  UW	bsearch;		/* この番号以下の block は使用中 */

  UW	datablock;		/* データ領域の開始位置。
				 * inode は、スーパーブ
				 * ロックの直後になるので、
				 * 特に記録しない。*/
};



struct sfs_inode
{
  UW	i_index;	/* SFS の場合、inode は 1 からはじまる */
  UW	i_nlink;
  UW	i_size;
  UW	i_nblock;

  UW	i_mode;
  UW	i_uid;
  UW	i_gid;

  SYSTIM	i_atime;
  SYSTIM	i_ctime;
  SYSTIM	i_mtime;

  UW	i_indirect[SFS_INDIRECT_BLOCK_ENTRY];
};


struct sfs_indirect
{
  UW	in_block[SFS_INDIRECT_BLOCK];
};


struct sfs_dir
{
  UW	d_index;		/* inode 番号 */
  B	d_name[SFS_MAXNAMELEN];
  B	pad[2];			/* padding */
};

#endif /* __FS_SFS_H__ */
