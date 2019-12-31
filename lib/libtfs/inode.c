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
#include <fs/config.h>
#include <sys/errno.h>
#include <sys/unistd.h>
#include "../libserv/libserv.h"
#include "func.h"

#ifndef ROUNDUP
#define ROUNDUP(x,align)	(((((int)x) + ((align) - 1))/(align))*(align))
#endif



/* inode に関係している処理
 *
 * sfs_read_inode()
 */


/* sfs_read_inode -
 *
 */
W sfs_read_inode(vfs_t *fsp, W ino, vnode_t *ip)
{
    struct tfs_inode *tfs_inode;

    tfs_inode = cache_get(&(fsp->device), ino);
    if (!tfs_inode) {
	return EIO;
    }

    ip->index = tfs_inode->i_inumber;
    ip->size = tfs_inode->i_size;
    ip->mode = tfs_inode->i_mode;
    ip->uid = tfs_inode->i_uid;
    ip->gid = tfs_inode->i_gid;
    ip->refer_count = 1;
    ip->lock_count = 0;
    ip->fs = fsp;
    ip->private = tfs_inode;

    return (0);
}


int sfs_stat(vnode_t *ip, struct stat *st)
{
    struct tfs *sb = (struct tfs *) (ip->fs->private);
    struct tfs_inode *tfs_inode = ip->private;

    st->st_dev = ip->fs->device.channel;
    st->st_ino = ip->index;
    st->st_mode = ip->mode;
    st->st_nlink = 1;
    st->st_size = ip->size;
    st->st_uid = ip->uid;
    st->st_gid = ip->gid;
    st->st_rdev = 0;
    st->st_blksize = sb->fs_bsize;
    st->st_blocks = ROUNDUP(st->st_size, st->st_blksize) / st->st_blksize;
    st->st_atime = tfs_inode->i_atime;
    st->st_mtime = tfs_inode->i_mtime;
    st->st_ctime = tfs_inode->i_ctime;

    return 0;
}

int sfs_i_close(vnode_t * ip)
{
    struct tfs_inode *tfs_inode = ip->private;
    if (!tfs_inode) {
	return 0;
    }

    tfs_inode->i_inumber = ip->index;
    if (ip->size < tfs_inode->i_size) {
      sfs_i_truncate(ip, ip->size);
    }

    tfs_inode->i_mode = ip->mode;
    tfs_inode->i_uid = ip->uid;
    tfs_inode->i_gid = ip->gid;
    tfs_inode->i_size = ip->size;

    if (ip->dirty) {
	if (!cache_modify(ip->private)) {
	    return EIO;
	}

	ip->dirty = false;
    }

    if (!cache_release(tfs_inode, false)) {
	return (EIO);
    }

    return 0;
}
