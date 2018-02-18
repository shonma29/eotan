/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* sfs_block.c - SFS のブロックの管理を行う
 *
 * $Log: sfs_block.c,v $
 * Revision 1.13  2000/07/02 04:14:22  naniwa
 * to implement disk block cache
 *
 * Revision 1.12  1999/07/09 08:13:42  naniwa
 * modified to cache bitmap block
 *
 * Revision 1.11  1999/06/01 09:18:56  naniwa
 * modified to make sfs_i_truncate () work
 *
 * Revision 1.10  1999/05/29 09:53:39  naniwa
 * removed variable 'sb'
 *
 * Revision 1.9  1999/05/28 15:48:31  naniwa
 * sfs ver 1.1
 *
 * Revision 1.8  1999/05/10 16:01:18  night
 * alloca を使って一時バッファの領域を確保している処理を、alloca の代わり
 * に動的な配列確保 (GCC の拡張機能) を使うように変更。
 *
 * Revision 1.7  1999/04/13 04:15:24  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.6  1999/03/24 04:52:13  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.5  1999/03/24 03:54:49  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.4  1997/10/24 14:00:39  night
 * 変数の引数合わせを厳密にした。
 *
 * Revision 1.3  1997/07/03 14:24:34  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.2  1996/11/18  13:46:10  night
 * ファイルの中身を作成。
 *
 * Revision 1.1  1996/11/17  14:52:56  night
 * 最初の登録
 *
 *
 */

#include <string.h>
#include <sys/errno.h>
#include "../libserv/libserv.h"
#include "func.h"


static W sfs_get_indirect_block_num (vfs_t *fsp,
				     struct sfs_inode *ip, W blockno);
static W sfs_set_indirect_block_num (vfs_t *fsp, 
				     struct sfs_inode *ip, W blockno, W newblock);

/* ブロックに関係している処理
 *
 * sfs_alloc_block()
 * sfs_get_block_num()
 * sfs_set_block_num()
 *
 */

/* sfs_alloc_block - ブロックをひとつアロケートする
 *
 */
W sfs_alloc_block(vfs_t * fsp)
{
    struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);
    W i, s;
    B *buf;

    if (sb->freeblock <= 0)
	return (-1);

    s = (sb->bsearch - 1) / (8 * sb->blksize);

    for (i = s; i < sb->bitmapsize; i++) {
	W j;

	buf = cache_get(&(fsp->device), i + 2);
	for (j = (i == s)? (((sb->bsearch - 1) / 8) % sb->blksize):0;
		j < sb->blksize; j++)
	    if (buf[j] & 0xff) {
		unsigned char mask = 1;
		W k;

		for (k = 0; k < 8; k++) {
		    if (mask & buf[j]) {
			W free_block = (i * sb->blksize * 8)
			    + (j * 8)
			    + k;

			buf[j] = buf[j] & ~mask;
			cache_release(buf, true);

			sb->freeblock--;
			sb->bsearch = free_block;

			if (!cache_modify(fsp->private))
			    return EIO;

			return (free_block);
		    }

		    mask = mask << 1;
		}
	    }

	cache_release(buf, false);
    }

    return (-1);
}


/* sfs_free_block - ブロックをひとつ解放する。
 *
 */
W sfs_free_block(vfs_t * fsp, W blockno)
{
    struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);
    W mask;
    int s;
    B *buf;

    if (blockno < sb->datablock) {
	dbg_printf("sfs: sfs_free_block: illegal block # %d\n", blockno);
	return (EINVAL);
    }

    s = blockno / (8 * sb->blksize);
    buf = cache_get(&(fsp->device), s + 2);
    s = (blockno / 8) % sb->blksize;

    mask = 0x01;
    mask = mask << (blockno % 8);
    buf[s] = buf[s] | (mask & 0xff);
    cache_release(buf, true);

    /* キャッシュに残っているデータを無効にする */
    cache_invalidate(&(fsp->device), blockno);

    sb->freeblock++;

    if (sb->bsearch >= blockno && blockno > 0)
	sb->bsearch = blockno - 1;

    if (!cache_modify(fsp->private))
	return EIO;

    return 0;
}

W sfs_free_indirect(vfs_t * fsp, struct sfs_inode * ip,
		    int offset, int inblock)
{
    int i;
    uint32_t *inbufp;

    if (offset != 0) {
	inbufp = cache_get(&(fsp->device), ip->i_indirect[inblock]);

	for (i = offset; i < num_of_2nd_blocks(fsp->device.block_size); ++i)
	    if (inbufp[i] > 0) {
		sfs_free_block(fsp, inbufp[i]);
		inbufp[i] = 0;
	    }

	cache_release(inbufp, true);
	++inblock;
    }

    for (i = inblock; i < num_of_1st_blocks(fsp->device.block_size); ++i) {
	if (ip->i_indirect[i] > 0) {
	    int j;

	    inbufp = cache_get(&(fsp->device), ip->i_indirect[i]);

	    for (j = 0; j < num_of_2nd_blocks(fsp->device.block_size); ++j)
		if (inbufp[j] > 0)
		    sfs_free_block(fsp, inbufp[j]);

	    sfs_free_block(fsp, ip->i_indirect[i]);
	}

	ip->i_indirect[i] = 0;
    }

    return 0;
}

/* sfs_get_block_num - ファイルのデータが実際のどのブロックにあるかを検索する。
 *
 */
W
sfs_get_block_num(vfs_t * fsp, struct sfs_inode * ip, W blockno)
{
    if (blockno < (num_of_1st_blocks(fsp->device.block_size)
	    * num_of_2nd_blocks(fsp->device.block_size)))
	/* 一重間接ブロックの範囲内
	 */
	return (sfs_get_indirect_block_num(fsp, ip, blockno));

    return (-1);
}


static W
sfs_get_indirect_block_num(vfs_t * fsp, struct sfs_inode * ip,
			   W blockno)
{
    size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
    W inblock = blockno / blocks;
    W inblock_offset = blockno % blocks;
    W bn;
    uint32_t *inbufp;

    if (ip->i_indirect[inblock] <= 0)
	return (-1);

    inbufp = cache_get(&(fsp->device), ip->i_indirect[inblock]);
    bn = inbufp[inblock_offset];
    cache_release(inbufp, false);

    return (bn);
}


W
sfs_set_block_num(vfs_t * fsp,
		  struct sfs_inode * ip, W blockno, W newblock)
{
    if (newblock < 0)
	return (-1);

    if (blockno < (num_of_1st_blocks(fsp->device.block_size)
	    * num_of_2nd_blocks(fsp->device.block_size)))
	/* 一重間接ブロックの範囲内
	 */
	return (sfs_set_indirect_block_num
		(fsp, ip, blockno, newblock));

    return (-1);
}



static W
sfs_set_indirect_block_num(vfs_t * fsp,
			   struct sfs_inode * ip, W blockno, W newblock)
{
    size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
    W inblock = blockno / blocks;
    W inblock_offset = blockno % blocks;
    uint32_t *inbufp;

    if (ip->i_indirect[inblock] <= 0) {
	W newinblock = sfs_alloc_block(fsp);

	ip->i_indirect[inblock] = newinblock;
	inbufp = cache_create(&(fsp->device), newinblock);

    } else
	inbufp = cache_get(&(fsp->device), ip->i_indirect[inblock]);

    inbufp[inblock_offset] = newblock;
    cache_release(inbufp, true);

    return (newblock);
}
