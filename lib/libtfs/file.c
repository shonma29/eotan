/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* sfs_file.c - SFS のファイルに関する処理を行う。
 *
 * $Log: sfs_file.c,v $
 * Revision 1.30  2000/07/02 04:14:23  naniwa
 * to implement disk block cache
 *
 * Revision 1.29  2000/05/20 11:56:24  naniwa
 * to support chdir
 *
 * Revision 1.28  2000/05/06 03:53:17  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.27  2000/02/20 09:38:07  naniwa
 * to work with fs_register_inode
 *
 * Revision 1.26  2000/02/06 09:14:10  naniwa
 * minor fix
 *
 * Revision 1.25  2000/01/28 10:04:43  naniwa
 * minor fix around strncpy, strncmp
 *
 * Revision 1.24  2000/01/26 08:25:07  naniwa
 * minor fix
 *
 * Revision 1.23  2000/01/08 09:12:17  naniwa
 * fixed sfs_i_unlink
 *
 * Revision 1.22  1999/12/26 11:18:21  naniwa
 * to implement unlink
 *
 * Revision 1.21  1999/07/09 08:13:44  naniwa
 * modified to cache bitmap block
 *
 * Revision 1.20  1999/06/01 09:18:58  naniwa
 * modified to make sfs_i_truncate () work
 *
 * Revision 1.19  1999/05/30 04:05:35  naniwa
 * modified to creat file correctly
 *
 * Revision 1.18  1999/05/29 09:53:41  naniwa
 * removed variable 'sb'
 *
 * Revision 1.17  1999/05/28 15:48:35  naniwa
 * sfs ver 1.1
 *
 * Revision 1.16  1999/05/12 17:19:08  night
 * sfs_i_create() の中身を作成。
 *
 * Revision 1.15  1999/05/10 16:07:07  night
 * alloca を使って一時バッファの領域を確保している処理を、alloca の代わり
 * に動的な配列確保 (GCC の拡張機能) を使うように変更。
 *
 * Revision 1.14  1999/04/13 04:15:27  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.13  1999/03/24 04:52:17  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.12  1997/10/11 16:25:21  night
 * ファイルの write 処理の修正。
 *
 * Revision 1.11  1997/09/23 13:54:29  night
 * デバッグ文の追加。
 *
 * Revision 1.10  1997/09/09 13:51:01  night
 * POSIX のファイルへの書き込み処理(システムコールは、write) を追加。
 *
 * Revision 1.9  1997/08/31 13:36:25  night
 * ファイルの sync 時、ファイルが書き換えられていた時のみ中身を書き出すように
 * 処理を変更した。
 * 以前は、常にファイルの中身を書き込むようにしていた。
 *
 * Revision 1.8  1997/07/07 14:42:31  night
 * sfs_i_close 実行時に EP_OK を返すように修正。
 * (ただし、処理としては何もしていない)
 *
 * Revision 1.7  1997/07/07 12:18:04  night
 * sfs_i_sync 関数の中身を追加。
 *
 * Revision 1.6  1997/07/04 15:07:41  night
 * ・スペシャルファイル - デバイスドライバポートの対応表の関連処理の追加。
 * ・ファイルの読み込み処理の改訂。
 *
 * Revision 1.5  1997/07/03 14:24:35  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.4  1996/11/27 15:42:53  night
 * 中身がまだない関数について EP_NOSUP を返すように修正した。
 *
 * Revision 1.3  1996/11/20 12:13:19  night
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
#include <mpu/memory.h>
#include <sys/errno.h>
#include "../libserv/libserv.h"
#include "func.h"

#ifndef MIN
#define MIN(x,y)		((x > y) ? y : x)
#endif




/*
 * ファイルを作成する。
 *
 * 1) 新しい inode をアロケート。
 * 2) 親ディレクトリにアロケートした新しい inode の情報を追加。
 *
 */
int
sfs_i_create(vnode_t * parent,
	     const char *fname,
	     W mode, struct permission * acc, vnode_t ** retip)
{
    vnode_t *newip;
    W error_no;
    W i_index;
    SYSTIM clock;

    /* 引数のチェック */
    newip = vnodes_create(parent);
    if (newip == NULL) {
	return (ENOMEM);
    }
    *retip = newip;

    /* 新しい sfs_inode をアロケート */
    i_index = sfs_alloc_inode(parent->fs, newip);
    if (i_index <= 0) {
	vnodes_remove(newip);
	return (ENOMEM);
    }

    /* 設定 */
    struct sfs_inode *sfs_inode = newip->private;
    memset(sfs_inode, 0, sizeof(*sfs_inode));
    time_get(&clock);
    newip->fs = parent->fs;
    newip->refer_count = 1;
    newip->dirty = true;
    newip->mode = mode | S_IFREG;
    newip->nlink = 1;
    newip->index = i_index;
    sfs_inode->i_uid = acc->uid;
    sfs_inode->i_gid = acc->gid;
    newip->dev = 0;
    newip->size = 0;
    sfs_inode->i_atime = clock;
    sfs_inode->i_ctime = clock;
    sfs_inode->i_mtime = clock;

    vnodes_append(newip);

    error_no = tfs_append_entry(parent, fname, newip);
    if (error_no) {
	sfs_free_inode(newip->fs, newip);
	vnodes_remove(newip);
	return (error_no);
    }

    return 0;
}

W sfs_i_truncate(vnode_t * ip, W newsize)
{
    int nblock, blockno, inblock, offset;
    vfs_t *fsp;
    struct sfs_inode *sfs_ip;
    SYSTIM clock;

    fsp = ip->fs;
    struct sfs_inode *sfs_inode = ip->private;
    sfs_ip = sfs_inode;
    struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);
    nblock = roundUp(newsize, sb->blksize) / sb->blksize;
    if (nblock < roundUp(sfs_ip->i_size, sb->blksize) / sb->blksize) {
	/* 余分なブロックを開放 */
	blockno = nblock;
	if (blockno < (num_of_1st_blocks(fsp->device.block_size)
		* num_of_2nd_blocks(fsp->device.block_size))) {
	    /* 一重間接ブロックの範囲内 */
	    inblock = blockno;
	    size_t blocks = num_of_2nd_blocks(fsp->device.block_size);
	    offset = inblock % blocks;
	    inblock = inblock / blocks;
	    tfs_deallocate_1st(fsp, sfs_ip, inblock, offset);
	}
    }

    ip->size = newsize;
    time_get(&clock);
    sfs_inode->i_mtime = clock;
    sfs_inode->i_ctime = clock;
    ip->dirty = true;

    return 0;
}
