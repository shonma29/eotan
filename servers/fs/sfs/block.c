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
#include "../fs.h"
#include "func.h"


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
W sfs_alloc_block(W fd, struct fs * fsp)
{
    W i, j, k, s;
    B *buf;
    W free_block;
    unsigned char mask;
    W error_no;
    W cn;

    if (fsp->fs_freeblock <= 0) {
	return (-1);
    }

    s = (fsp->fs_bsearch - 1) / (8 * fsp->fs_blksize);
    for (i = s; i < fsp->fs_private.sfs_fs.sfs_bitmapsize; i++) {
	get_cache(fd, i + 2, &cn, &buf);
	if (i == s)
	    j = ((fsp->fs_bsearch - 1) / 8) % fsp->fs_blksize;
	else
	    j = 0;
	for (; j < fsp->fs_blksize; j++) {
	    if ((buf[j] & 0xff) != 0xff) {
		mask = 1;
		for (k = 0; k < 8; k++) {
		    if ((mask & buf[j]) != mask) {
			free_block = (i * fsp->fs_blksize * 8)
			    + (j * 8)
			    + k;
			buf[j] = buf[j] | mask;
			put_cache(cn, 1);

			fsp->fs_freeblock--;
			fsp->fs_usedblock++;
			fsp->fs_bsearch = free_block;
			fsp->fs_dirty = 1;
			/* ここで fs を sync する必要があるか? */
			error_no = sfs_syncfs(fsp, 0);
			if (error_no) {
			    return (-1);
			}
			return (free_block);
		    }
		    mask = mask << 1;
		}
	    }
	}
    }
    put_cache(cn, 0);
    return (-1);
}


/* sfs_free_block - ブロックをひとつ解放する。
 *
 */
W sfs_free_block(W fd, struct fs * fsp, W blockno)
{
    W mask;
    int s;
    B *buf;
    W cn;

    if (blockno < fsp->fs_private.sfs_fs.sfs_datablock) {
	dbg_printf("sfs: sfs_free_block: illegal block # %d\n", blockno);
	return (EINVAL);
    }

    s = blockno / (8 * fsp->fs_blksize);
    get_cache(fd, s + 2, &cn, &buf);
    s = (blockno / 8) % fsp->fs_blksize;

    mask = 0x01;
    mask = mask << (blockno % 8);
    buf[s] = buf[s] & ((~mask) & 0xff);
    put_cache(cn, 1);

    /* キャッシュに残っているデータを無効にする */
    check_cache(fd, blockno, &cn);
    if (cn > 0) {
	put_cache(cn, -1);
    }

    fsp->fs_freeblock++;
    fsp->fs_usedblock--;
    fsp->fs_dirty = 1;
    if (fsp->fs_bsearch >= blockno && blockno > 0)
	fsp->fs_bsearch = blockno - 1;
    /* ここで fs は sync しない 
       error_no = sfs_syncfs(fsp, 0);
       if (error_no)
       {
       return (EP_IO);
       }
     */
    return (EOK);
}

W sfs_free_indirect(W fd, struct fs * fsp, struct sfs_inode * ip,
		    int offset, int inblock)
{
    int i, j;
    W cn;
    struct sfs_indirect *inbufp;

    if (offset != 0) {
	get_cache(fd, ip->sfs_i_indirect[inblock], &cn,
		      (B **) & inbufp);
	for (i = offset; i < SFS_INDIRECT_BLOCK; ++i) {
	    if (inbufp->sfs_in_block[i] > 0) {
		sfs_free_block(fd, fsp, inbufp->sfs_in_block[i]);
		inbufp->sfs_in_block[i] = 0;
	    }
	}

	put_cache(cn, 1);
	++inblock;
    }
    for (i = inblock; i < SFS_INDIRECT_BLOCK_ENTRY; ++i) {
	if (ip->sfs_i_indirect[i] > 0) {
	    get_cache(fd, ip->sfs_i_indirect[i], &cn, (B **) & inbufp);
	    for (j = 0; j < SFS_INDIRECT_BLOCK; ++j) {
		if (inbufp->sfs_in_block[j] > 0) {
		    sfs_free_block(fd, fsp, inbufp->sfs_in_block[j]);
		}
	    }
	    put_cache(cn, 0);
	    sfs_free_block(fd, fsp, ip->sfs_i_indirect[i]);
	}
	ip->sfs_i_indirect[i] = 0;
    }
    return (EOK);
}

W sfs_free_dindirect(W fd, struct fs * fsp, struct sfs_inode * ip,
		     int offset, int dinblock, int inblock)
{
    int i, j;
    W cn1, cn2;
    struct sfs_indirect *inbufp, *inbufp2;

    if (ip->sfs_i_dindirect[inblock] <= 0) {
	return (EOK);
    }

    get_cache(fd, ip->sfs_i_dindirect[inblock], &cn1, (B **) & inbufp);

    if (offset != 0) {
	get_cache(fd, inbufp->sfs_in_block[dinblock], &cn2,
		      (B **) & inbufp2);
	for (i = offset; i < SFS_INDIRECT_BLOCK; ++i) {
	    if (inbufp2->sfs_in_block[i] > 0) {
		sfs_free_block(fd, fsp, inbufp2->sfs_in_block[i]);
		inbufp2->sfs_in_block[i] = 0;
	    }
	}

	put_cache(cn2, 1);
	++dinblock;
    }
    for (i = dinblock; i < SFS_INDIRECT_BLOCK; ++i) {
	if (inbufp->sfs_in_block[i] > 0) {
	    get_cache(fd, inbufp->sfs_in_block[i], &cn2,
			  (B **) & inbufp2);
	    for (j = 0; j < SFS_INDIRECT_BLOCK; ++j) {
		if (inbufp2->sfs_in_block[j] > 0)
		    sfs_free_block(fd, fsp, inbufp2->sfs_in_block[j]);
	    }

	    sfs_free_block(fd, fsp, inbufp->sfs_in_block[i]);
	    inbufp->sfs_in_block[i] = 0;
	    put_cache(cn2, 0);
	}
    }
    if (dinblock > 0) {
	put_cache(cn1, 1);
    } else
	put_cache(cn1, 0);
    return (EOK);
}

W sfs_free_all_dindirect(W fd, struct fs * fsp,
			 struct sfs_inode * ip, int inblock)
{
    int i;

    for (i = inblock; i < SFS_DINDIRECT_BLOCK_ENTRY; ++i) {
	if (ip->sfs_i_dindirect[i] > 0) {
	    sfs_free_dindirect(fd, fsp, ip, 0, 0, i);
	    if (ip->sfs_i_dindirect[i] > 0)
		sfs_free_block(fd, fsp, ip->sfs_i_dindirect[i]);
	    ip->sfs_i_dindirect[i] = 0;
	}
    }
    return (EOK);
}


/* sfs_get_block_num - ファイルのデータが実際のどのブロックにあるかを検索する。
 *
 */
W
sfs_get_block_num(W fd, struct fs * fsp, struct sfs_inode * ip, W blockno)
{
    if (blockno < SFS_DIRECT_BLOCK_ENTRY) {
	/* 直接ブロックの範囲内
	 */
	return (ip->sfs_i_direct[blockno]);
    }
	else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
			    +
			    (SFS_INDIRECT_BLOCK_ENTRY *
			     SFS_INDIRECT_BLOCK))) {
	/* 一重間接ブロックの範囲内
	 */
	return (sfs_get_indirect_block_num(fd, fsp, ip, blockno));
    }
	else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
			    +
			    (SFS_INDIRECT_BLOCK_ENTRY *
			     SFS_INDIRECT_BLOCK) +
			    (SFS_DINDIRECT_BLOCK_ENTRY *
			     SFS_INDIRECT_BLOCK * SFS_INDIRECT_BLOCK))) {
	/* 二重間接ブロックの範囲内
	 */
	return (sfs_get_dindirect_block_num(fd, fsp, ip, blockno));
    }

    return (-1);
}


W
sfs_get_indirect_block_num(W fd, struct fs * fsp, struct sfs_inode * ip,
			   W blockno)
{
    W inblock;
    W inblock_offset;
    W cn, bn;
    struct sfs_indirect *inbufp;

    inblock = (blockno - SFS_DIRECT_BLOCK_ENTRY);
    inblock_offset = inblock % SFS_INDIRECT_BLOCK;
    inblock = inblock / SFS_INDIRECT_BLOCK;
    if (ip->sfs_i_indirect[inblock] <= 0) {
	return (-1);
    }

    get_cache(fd, ip->sfs_i_indirect[inblock], &cn, (B **) & inbufp);
    bn = inbufp->sfs_in_block[inblock_offset];
    put_cache(cn, 0);
    return (bn);
}


W
sfs_get_dindirect_block_num(W fd, struct fs * fsp, struct sfs_inode * ip,
			    W blockno)
{
    W dinblock;
    W dinblock_offset;
    W inblock;
    W cn, bn;
    struct sfs_indirect *inbufp;

    blockno =
	blockno - (SFS_DIRECT_BLOCK_ENTRY +
		   SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);

    inblock = blockno / (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);
    dinblock =
	(blockno % (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)) /
	SFS_INDIRECT_BLOCK;
    dinblock_offset = blockno % SFS_INDIRECT_BLOCK;

#ifdef FMDEBUG
    dbg_printf
	("sfs: GET: blockno = %d, inblock = %d, dinblock = %d, dinblock_offset = %d\n",
	 blockno, inblock, dinblock, dinblock_offset);
#endif
    if (ip->sfs_i_dindirect[inblock] <= 0) {
	return (-1);
    }

    get_cache(fd, ip->sfs_i_dindirect[inblock], &cn, (B **) & inbufp);
    bn = inbufp->sfs_in_block[dinblock];
    put_cache(cn, 0);
    if (bn <= 0) {
	return (-1);
    }

    get_cache(fd, bn, &cn, (B **) & inbufp);
    bn = inbufp->sfs_in_block[dinblock_offset];
    put_cache(cn, 0);
    return (bn);
}


W
sfs_set_block_num(W fd,
		  struct fs * fsp,
		  struct sfs_inode * ip, W blockno, W newblock)
{
    if (newblock < 0) {
	return (-1);
    }
    else if (newblock < fsp->fs_private.sfs_fs.sfs_datablock) {
	dbg_printf("sfs: illegal newblock %d %d\b", blockno, newblock);
    }

    if (blockno < (SFS_DIRECT_BLOCK_ENTRY)) {
	/* 直接ブロックの範囲内
	 */
	ip->sfs_i_direct[blockno] = newblock;
	return (ip->sfs_i_direct[blockno]);
    } else if (blockno < (SFS_DIRECT_BLOCK_ENTRY
			  +
			  (SFS_INDIRECT_BLOCK_ENTRY *
			   SFS_INDIRECT_BLOCK))) {
	/* 一重間接ブロックの範囲内
	 */
	return (sfs_set_indirect_block_num
		(fd, fsp, ip, blockno, newblock));
    } else if (blockno <
	       (SFS_DIRECT_BLOCK_ENTRY +
		(SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK) +
		(SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK *
		 SFS_INDIRECT_BLOCK))) {
	/* 二重間接ブロックの範囲内
	 */
	return (sfs_set_dindirect_block_num
		(fd, fsp, ip, blockno, newblock));
    }

    return (-1);
}



W
sfs_set_indirect_block_num(W fd,
			   struct fs * fsp,
			   struct sfs_inode * ip, W blockno, W newblock)
{
    W inblock;
    W inblock_offset;
    W newinblock;
    W cn;
    struct sfs_indirect *inbufp;

    inblock = (blockno - SFS_DIRECT_BLOCK_ENTRY);
    inblock_offset = inblock % SFS_INDIRECT_BLOCK;
    inblock = inblock / SFS_INDIRECT_BLOCK;
    if (ip->sfs_i_indirect[inblock] <= 0) {
	newinblock = sfs_alloc_block(fd, fsp);
	ip->sfs_i_indirect[inblock] = newinblock;
	get_cache(fd, newinblock, &cn, (B **) & inbufp);
	memset((B*)inbufp, 0, sizeof(struct sfs_indirect));
    } else {
	get_cache(fd, ip->sfs_i_indirect[inblock], &cn,
		      (B **) & inbufp);
    }

    inbufp->sfs_in_block[inblock_offset] = newblock;
    put_cache(cn, 1);

    /* inode の書き込みは sfs_i_write() 以上の部分で行われるはず...
       sfs_write_inode (fd, fsp, ip);
     */

#ifdef FMDEBUG
    if (blockno == 440)
	dbg_printf
	    ("sfs: set_ind: blockno = %d cache = %d inblock = %d, offset = %d, newblock = %d\n",
	     ip->sfs_i_indirect[inblock], cn, inblock, inblock_offset,
	     newblock);
#endif

    return (newblock);
}


/* sfs_set_dindirect_block_num - 
 *
 */
W
sfs_set_dindirect_block_num(W fd,
			    struct fs * fsp,
			    struct sfs_inode * ip, W blockno, W newblock)
{
    W dinblock;
    W dinblock_offset;
    W inblock;
    W newinblock;
    W newdinblock;
    W cn1, cn2;
    struct sfs_indirect *inbufp;	/* 一番目の間接ブロックの情報 */
    struct sfs_indirect *dinbufp;	/* 二番目の間接ブロックの情報 */

    blockno =
	blockno - (SFS_DIRECT_BLOCK_ENTRY +
		   (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK));

    inblock = blockno / (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK);
    dinblock =
	(blockno % (SFS_DINDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)) /
	SFS_INDIRECT_BLOCK;
    dinblock_offset = blockno % SFS_INDIRECT_BLOCK;

    if (ip->sfs_i_dindirect[inblock] <= 0) {
	/* 一重目の間接ブロックの更新(アロケート)
	 */
	newinblock = sfs_alloc_block(fd, fsp);
	ip->sfs_i_dindirect[inblock] = newinblock;
	get_cache(fd, newinblock, &cn1, (B **) & inbufp);
	memset((B*)inbufp, 0, sizeof(struct sfs_indirect));
    } else {
	get_cache(fd, ip->sfs_i_dindirect[inblock], &cn1,
		      (B **) & inbufp);
    }

    if (inbufp->sfs_in_block[dinblock] <= 0) {
	/* 二番目の間接ブロックの更新
	 * (アロケート)
	 */
	newdinblock = sfs_alloc_block(fd, fsp);
	inbufp->sfs_in_block[dinblock] = newdinblock;
	put_cache(cn1, 1);
	get_cache(fd, newdinblock, &cn2, (B **) & dinbufp);
	memset((B*)dinbufp, 0, sizeof(struct sfs_indirect));
    } else {
	put_cache(cn1, 0);
	get_cache(fd, inbufp->sfs_in_block[dinblock], &cn2,
		      (B **) & dinbufp);
    }

    dinbufp->sfs_in_block[dinblock_offset] = newblock;

    put_cache(cn2, 1);
    return (newblock);
}
