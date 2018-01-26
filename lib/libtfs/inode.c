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
#include <fs/nconfig.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include "../libserv/libserv.h"
#include "func.h"

#ifndef ROUNDUP
#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#endif

static W	sfs_get_inode_offset (vfs_t *fsp, W ino);



/* inode に関係している処理
 *
 * sfs_get_inode_offset()
 * sfs_read_inode()
 * sfs_alloc_inode()
 */


/* get_inode_offset -
 *
 */
static W sfs_get_inode_offset(vfs_t *fsp, W ino)
{
    W offset;
    W nblock;
    W blocksize;
    struct sfs_superblock *sb;

    sb = (struct sfs_superblock*)(fsp->private);
    nblock = sb->nblock;
    blocksize = sb->blksize;
    offset = 1 + 1 + (ROUNDUP(nblock / 8, blocksize) / blocksize);
    offset *= blocksize;
    return (offset + ((ino - 1) * sizeof(struct sfs_inode)));
}


/* sfs_read_inode -
 *
 */
W sfs_read_inode(vfs_t *fsp, W ino, vnode_t *ip)
{
    W offset;
    struct sfs_inode *sfs_inode;

    offset = sfs_get_inode_offset(fsp, ino);
    sfs_inode = cache_get(&(fsp->device), offset / SFS_BLOCK_SIZE);
    if (!sfs_inode) {
	return EIO;
    }

    ip->index = sfs_inode->i_index;
    ip->nlink = sfs_inode->i_nlink;
    ip->size = sfs_inode->i_size;
    ip->nblock = sfs_inode->i_nblock;
    ip->mode = sfs_inode->i_mode;
    ip->uid = sfs_inode->i_uid;
    ip->gid = sfs_inode->i_gid;
    ip->refer_count = 1;
    ip->lock_count = 0;
    ip->fs = fsp;
    ip->dev =  0;
    ip->private = sfs_inode;

    return (0);
}


/* sfs_alloc_inode -
 *
 */
W sfs_alloc_inode(vfs_t * fsp, vnode_t *ip)
{
    W i;
    W offset;
    struct sfs_inode *ipbufp;
    struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);

    if (sb->freeinode <= 0) {
	return (0);
    }

    offset = sfs_get_inode_offset(fsp, sb->isearch);
    for (i = sb->isearch; i <= sb->ninode; i++) {
	ipbufp = cache_get(&(fsp->device), offset / SFS_BLOCK_SIZE);
	if (!ipbufp) {
	    return EIO;
	}

	offset += sizeof(struct sfs_inode);
	if (ipbufp->i_index != i) {
	    fsp->device.clear(&(fsp->device), (VP)ipbufp);
	    ipbufp->i_index = i;
	    if (!cache_modify(ipbufp)) {
		cache_release(ipbufp, false);
		return EIO;
	    }

	    ip->private = ipbufp;
	    sb->freeinode--;
	    sb->isearch = (i + 1);

	    if (!cache_modify(fsp->private)) {
		ip->private = NULL;
		cache_release(ipbufp, false);
		return EIO;
	    }

	    return (i);
	}

	cache_release(ipbufp, false);
    }

    return (0);
}


/* sfs_free_inode -
 *
 */
W sfs_free_inode(vfs_t * fsp, vnode_t *ip)
{
    W inode_index = ip->index;
    fsp->device.clear(&(fsp->device), ip->private);
    if (!cache_modify(ip->private)) {
	return EIO;
    }
    ip->dirty = false;

    struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);
    sb->freeinode++;
    if (sb->isearch >= inode_index)
	sb->isearch = inode_index;

    if (!cache_modify(fsp->private))
	return EIO;

    return 0;
}

int sfs_stat(vnode_t *ip, struct stat *st)
{
    struct sfs_superblock *sb = (struct sfs_superblock*)(ip->fs->private);
    struct sfs_inode *sfs_inode = ip->private;

    st->st_dev = ip->fs->device.channel;
    st->st_ino = ip->index;
    st->st_mode = ip->mode;
    st->st_nlink = ip->nlink;
    st->st_size = ip->size;
    st->st_uid = ip->uid;
    st->st_gid = ip->gid;
    st->st_rdev = ip->dev;
    st->st_blksize = sb->blksize;
    st->st_blocks = ROUNDUP(st->st_size, st->st_blksize) / st->st_blksize;
    st->st_atime = sfs_inode->i_atime.sec;
    st->st_mtime = sfs_inode->i_mtime.sec;
    st->st_ctime = sfs_inode->i_ctime.sec;

    return 0;
}

int sfs_wstat(vnode_t *ip)
{
    struct sfs_inode *sfs_inode = ip->private;
    sfs_inode->i_mode = ip->mode;
    sfs_inode->i_gid = ip->gid;
    time_get(&(sfs_inode->i_ctime));
    ip->dirty = true;

    return 0;
}

/*
 * permit -
 */
//TODO check fs is readonly
int sfs_permit(vnode_t * ip, struct permission * acc, UW bits)
{
    UW mode, perm_bits;
    int shift;

    mode = ip->mode;
    if (acc->uid == ROOT_UID) {
	if (((mode & S_IFMT) == S_IFDIR) ||
	    (mode & (X_OK << 6 | X_OK << 3 | X_OK))) {
	    perm_bits = R_OK | W_OK | X_OK;
	} else {
	    perm_bits = R_OK | W_OK;
	}
    } else {
	if (acc->uid == ip->uid)
	    shift = 6;
	else if (acc->gid == ip->gid)
	    shift = 3;
	else
	    shift = 0;
	perm_bits = (mode >> shift) & 0x03;
    }

    if ((perm_bits | bits) != perm_bits)
	return (EACCES);
    return 0;
}


int sfs_i_close(vnode_t * ip)
{
    W err;

#ifdef FMDEBUG
    dbg_printf("sfs: sfs_i_close\n");
#endif
    struct sfs_inode *sfs_inode = ip->private;
    if (!sfs_inode) {
	return 0;
    }

    sfs_inode->i_index = ip->index;
    if (ip->size < sfs_inode->i_size) {
      sfs_i_truncate(ip, ip->size);
    }

    sfs_inode->i_nlink = ip->nlink;
    sfs_inode->i_mode = ip->mode;
    sfs_inode->i_gid = ip->gid;
    sfs_inode->i_size = ip->size;
    sfs_inode->i_nblock = ip->nblock;

    if (ip->dirty) {
	if (!cache_modify(ip->private)) {
	    return EIO;
	}

	ip->dirty = false;
    }

    err = cache_release(sfs_inode, false);
    if (err) {
	return (err);
    }

#ifdef FMDEBUG
    dbg_printf("sfs: sfs_i_close: done\n");
#endif

    return 0;
}
