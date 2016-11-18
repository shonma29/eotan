/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* sfs_inode.c - SFS の inode の管理を行う
 *
 * $Log: sfs_inode.c,v $
 * Revision 1.15  2000/07/02 04:14:26  naniwa
 * to implement disk block cache
 *
 * Revision 1.14  2000/05/06 03:53:20  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.13  2000/02/20 09:38:09  naniwa
 * to work with fs_register_inode
 *
 * Revision 1.12  1999/12/26 11:18:23  naniwa
 * to implement unlink
 *
 * Revision 1.11  1999/12/23 07:03:46  naniwa
 * fixed sfs_alloc_inode
 *
 * Revision 1.10  1999/12/21 10:55:46  naniwa
 * fixed sfs_alloc_inode ()
 *
 * Revision 1.9  1999/05/28 15:48:39  naniwa
 * sfs ver 1.1
 *
 * Revision 1.8  1999/05/10 15:55:02  night
 * inode の書き込み時に、inode のサイズが 1024 バイト以上のときには、1024
 * 毎に書き込むように処理を変更。ただし、2048 以上の場合はサポートしてい
 * ない。
 *
 * Revision 1.7  1997/10/24 14:00:48  night
 * 変数の引数合わせを厳密にした。
 *
 * Revision 1.6  1997/07/07 15:25:33  night
 * ROUNDUP マクロがすでに定義しているならば、再定義しないように修正した。
 *
 * Revision 1.5  1997/07/04 15:07:41  night
 * ・スペシャルファイル - デバイスドライバポートの対応表の関連処理の追加。
 * ・ファイルの読み込み処理の改訂。
 *
 * Revision 1.4  1997/07/03 14:24:37  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.3  1996/11/20  12:13:19  night
 * 細々とした修正。
 *
 * Revision 1.2  1996/11/18  13:46:11  night
 * ファイルの中身を作成。
 *
 * Revision 1.1  1996/11/17  14:52:57  night
 * 最初の登録
 *
 *
 */

#include <string.h>
#include "../fs.h"
#include "func.h"

#ifndef ROUNDUP
#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#endif

static W	sfs_get_inode_offset (struct fs *fsp, W ino);



/* inode に関係している処理
 *
 * sfs_get_inode_offset()
 * sfs_read_inode()
 * sfs_alloc_inode()
 * sfs_write_inode()
 */


/* get_inode_offset -
 *
 */
static W sfs_get_inode_offset(struct fs *fsp, W ino)
{
    W offset;
    W nblock;
    W blocksize;
    struct sfs_superblock *sb;

    sb = &(fsp->private.sfs_fs);
    nblock = sb->nblock;
    blocksize = sb->blksize;
    offset = 1 + 1 + (ROUNDUP(nblock / 8, blocksize) / blocksize);
    offset *= blocksize;
    return (offset + ((ino - 1) * sizeof(struct sfs_inode)));
}


/* sfs_read_inode -
 *
 */
W sfs_read_inode(struct fs *fsp, W ino, struct inode *ip)
{
    W offset;
    ID fd;
    W cn;
    B *buf;

    fd = fsp->device;
    offset = sfs_get_inode_offset(fsp, ino);
    get_cache(fd, offset / SFS_BLOCK_SIZE, &cn, &buf);
    memcpy((B*)&(ip->i_private.sfs_inode), buf,
	  sizeof(struct sfs_inode));
    put_cache(cn, 0);

    ip->i_index = ip->i_private.sfs_inode.i_index;
    ip->i_size = ip->i_private.sfs_inode.i_size;
    ip->i_nblock = ip->i_private.sfs_inode.i_nblock;
    ip->i_mode = ip->i_private.sfs_inode.i_mode;
    ip->i_refcount = 1;
    ip->i_lock = 0;
    ip->i_fs = fsp;
    ip->i_dev =  0;

    return (0);
}


/* sfs_alloc_inode -
 *
 */
W sfs_alloc_inode(ID fd, struct fs * fsp)
{
    W i;
    W offset;
    struct sfs_inode *ipbufp;
    W cn;

    if (fsp->private.sfs_fs.freeinode <= 0) {
	return (0);
    }

    offset = sfs_get_inode_offset(fsp, fsp->private.sfs_fs.isearch);
    for (i = fsp->private.sfs_fs.isearch; i <= fsp->private.sfs_fs.ninode; i++) {
	get_cache(fd, offset / SFS_BLOCK_SIZE, &cn, (B **) & ipbufp);

	offset += sizeof(struct sfs_inode);
	if (ipbufp->i_index != i) {
	    memset((VP)ipbufp, 0, sizeof(struct sfs_inode));
	    ipbufp->i_index = i;
	    put_cache(cn, 1);
	    fsp->private.sfs_fs.freeinode--;
	    fsp->private.sfs_fs.isearch = (i + 1);
	    fsp->dirty = 1;
	    /* ここで fs の sync を行う必要があるか? */
	    sfs_syncfs(fsp, 0);
	    return (i);
	} else {
	    put_cache(cn, 0);
	}
    }

    return (0);
}


/* sfs_write_inode -
 *
 */
W sfs_write_inode(W fd, struct fs * fsp, struct sfs_inode * ip)
{
    W error_no;
    W rlength;
    W cn;
    B *buf;

    if (sizeof(struct sfs_inode) >= MAX_BODY_SIZE) {	/* Kludge!! */
	B *tmp;

	tmp = (B *) ip;
	error_no = write_device(fd,
				 tmp,
				 sfs_get_inode_offset(fsp,
						      ip->i_index),
				 MAX_BODY_SIZE, &rlength);
	if (error_no) {
	    return (EIO);
	}

	tmp += MAX_BODY_SIZE;
	error_no = write_device(fd,
				 tmp,
				 sfs_get_inode_offset(fsp, ip->i_index)
				 + MAX_BODY_SIZE,
				 sizeof(struct sfs_inode) - MAX_BODY_SIZE,
				 &rlength);

	if (error_no) {
	    return (EIO);
	}
    } else {
	get_cache(fd,
		      sfs_get_inode_offset(fsp,
					   ip->i_index) /
		      SFS_BLOCK_SIZE, &cn, &buf);
	memcpy(buf, (B*)ip, sizeof(struct sfs_inode));
	put_cache(cn, 1);
	/* ここで fs の sync を行う必要があるか? */
	sfs_syncfs(fsp, 0);
    }

    return (EOK);
}


/* sfs_free_inode -
 *
 */
W sfs_free_inode(struct fs * fsp, struct inode *ip)
{
    W inode_index;
    W cn;
    B *buf;

    inode_index = ip->i_index;
    get_cache(fsp->device,
		  sfs_get_inode_offset(fsp, inode_index) / SFS_BLOCK_SIZE,
		  &cn, &buf);
    memset(buf, 0, sizeof(struct sfs_inode));
    put_cache(cn, 1);
    ip->i_dirty = 0;

    fsp->private.sfs_fs.freeinode++;
    fsp->dirty = 1;
    if (fsp->private.sfs_fs.isearch >= inode_index)
	fsp->private.sfs_fs.isearch = inode_index;
    /* ここで fs の sync を行う必要があるか? */
    sfs_syncfs(fsp, 0);
    return (EOK);
}

W sfs_stat(struct inode *ip, struct stat *st)
{
    st->st_dev = ip->i_fs->device;
    st->st_ino = ip->i_index;
    st->st_mode = ip->i_mode;
    st->st_nlink = ip->i_private.sfs_inode.i_nlink;
    st->st_size = ip->i_size;
    st->st_uid = ip->i_private.sfs_inode.i_uid;
    st->st_gid = ip->i_private.sfs_inode.i_gid;
    st->st_rdev = ip->i_dev;
    st->st_blksize = ip->i_fs->private.sfs_fs.blksize;
    st->st_blocks = ROUNDUP(st->st_size, st->st_blksize) / st->st_blksize;
    st->st_atime = ip->i_private.sfs_inode.i_atime.sec;
    st->st_mtime = ip->i_private.sfs_inode.i_mtime.sec;
    st->st_ctime = ip->i_private.sfs_inode.i_ctime.sec;

    return (EOK);
}

W sfs_wstat(struct inode *ip)
{
    time_get(&(ip->i_private.sfs_inode.i_ctime));

    return (EOK);
}

/*
 * permit -
 */

W sfs_permit(struct inode * ip, struct permission * acc, UW bits)
{
    UW mode, perm_bits;
    int shift;

    mode = ip->i_mode;
    if (acc->uid == SU_UID) {
	if (((mode & S_IFMT) == S_IFDIR) ||
	    (mode & (X_OK << 6 | X_OK << 3 | X_OK))) {
	    perm_bits = R_OK | W_OK | X_OK;
	} else {
	    perm_bits = R_OK | W_OK;
	}
    } else {
	if (acc->uid == ip->i_private.sfs_inode.i_uid)
	    shift = 6;
	else if (acc->gid == ip->i_private.sfs_inode.i_gid)
	    shift = 3;
	else
	    shift = 0;
	perm_bits = (mode >> shift) & 0x03;
    }

    if ((perm_bits | bits) != perm_bits)
	return (EACCES);
    return (EOK);
}


W sfs_i_sync(struct inode * ip)
{
    W err;

#ifdef FMDEBUG
    dbg_printf("sfs: sfs_i_sync\n");
#endif
    ip->i_private.sfs_inode.i_index = ip->i_index;
    if (ip->i_size < ip->i_private.sfs_inode.i_size) {
      sfs_i_truncate(ip, ip->i_size);
    }
    ip->i_private.sfs_inode.i_size = ip->i_size;
    ip->i_private.sfs_inode.i_nblock = ip->i_nblock;
    ip->i_private.sfs_inode.i_mode = ip->i_mode;

    if (ip->i_dirty) {
	err = sfs_write_inode(ip->i_fs->device, ip->i_fs,
			      &(ip->i_private.sfs_inode));
	if (err) {
	    return (err);
	}
    }
    ip->i_dirty = 0;

#ifdef FMDEBUG
    dbg_printf("sfs: sfs_i_sync: done\n");
#endif
    return (EOK);
}

