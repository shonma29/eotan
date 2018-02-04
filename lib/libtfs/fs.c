/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* sfs_fs.c - SFS のファイルシステム全体の処理
 *
 *
 * $Log: sfs_fs.c,v $
 * Revision 1.23  2000/07/02 04:14:24  naniwa
 * to implement disk block cache
 *
 * Revision 1.22  2000/06/01 08:47:25  naniwa
 * to implement getdents
 *
 * Revision 1.21  2000/05/06 03:53:18  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.20  1999/12/19 11:03:20  naniwa
 * minor fix of debug message
 *
 * Revision 1.19  1999/11/19 10:10:54  naniwa
 * add debug message
 *
 * Revision 1.18  1999/07/09 08:13:45  naniwa
 * modified to cache bitmap block
 *
 * Revision 1.17  1999/05/28 15:48:36  naniwa
 * sfs ver 1.1
 *
 * Revision 1.16  1999/04/13 04:15:28  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.15  1999/03/24 04:52:18  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.14  1997/10/11 16:25:22  night
 * ファイルの write 処理の修正。
 *
 * Revision 1.13  1997/08/31 13:37:05  night
 * root file system の設定時、Magic ID が異なっていた時にエラーを
 * 返すように変更した。
 *
 * Revision 1.12  1997/07/06 11:57:53  night
 * デバッグ文の修正。
 *
 * Revision 1.11  1997/07/03 14:24:36  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.10  1997/07/02 13:26:39  night
 * sfs_mountroot の引数が間違っていたので、修正した
 *
 * Revision 1.9  1997/05/08 15:13:08  night
 * sfs_mountroot() 実行時に、rootfile->i_ops に対して sfs_iops を
 * 設定する処理を有効にした。
 *
 * Revision 1.8  1997/04/28 15:29:53  night
 * デバッグ用の文を追加。
 *
 * Revision 1.7  1997/04/25 13:02:36  night
 * mountroot 実行時にファイルシステム情報の設定処理を追加した。
 *
 * Revision 1.6  1997/04/24 15:43:34  night
 * sfs_mountroot の実行時に正常に終了した場合には EP_OK のエラーコードを
 * 返すように修正。
 *
 * Revision 1.5  1996/11/20  12:13:19  night
 * 細々とした修正。
 *
 * Revision 1.4  1996/11/18  13:46:11  night
 * ファイルの中身を作成。
 *
 * Revision 1.3  1996/11/17  14:51:24  night
 * rcsid の追加。
 *
 * Revision 1.2  1996/11/07  21:12:11  night
 * 文字コードを EUC に変更した。
 *
 * Revision 1.1  1996/11/07  12:50:56  night
 * 最初の登録
 *
 */

#include <fstype.h>
#include <string.h>
#include <sys/errno.h>
#include "func.h"
#include "../libserv/libserv.h"


static int sfs_mount (ID device, vfs_t *rootfsp, vnode_t *rootfile);
static int sfs_unmount(vfs_t * rootfsp);
static int sfs_statvfs(vfs_t *, struct statvfs *);

vfs_operation_t sfs_fsops = {
    sfs_mount,
    sfs_unmount,
    sfs_syncfs,
    sfs_statvfs,
    tfs_getdents,
    tfs_walk,
    sfs_i_link,
    sfs_i_unlink,
    tfs_mkdir,
    sfs_i_rmdir,
    sfs_stat,
    sfs_wstat,
    sfs_i_create,
    sfs_i_close,
    sfs_i_read,
    sfs_i_write
};




/* sfs_mount -
 *
 */
static int sfs_mount(ID device, vfs_t *rootfsp, vnode_t *rootfile)
{
    block_initialize(&(rootfsp->device));
    rootfsp->device.channel = device;
    rootfsp->device.block_size = SFS_BLOCK_SIZE;
    rootfsp->private = cache_get(&(rootfsp->device), 1);

    if (!(rootfsp->private))
	return (EIO);

    struct sfs_superblock *sfs_sb = rootfsp->private;
    if (sfs_sb->magic != SFS_MAGIC) {
	dbg_printf("sfs: ERROR: mount: magic number %x\n", sfs_sb->magic);
	cache_release(rootfsp->private, false);
	return (EINVAL);
    }
    if (sfs_sb->blksize != SFS_BLOCK_SIZE) {
	dbg_printf("sfs: ERROR: mount: block size %d\n", sfs_sb->blksize);
	cache_release(rootfsp->private, false);
	return (EINVAL);
    }

    /* root file の読み込み、inode = 1 が root file */
    W error_no = sfs_read_inode(rootfsp, 1, rootfile);
    if (error_no) {
	cache_release(rootfsp->private, false);
	return (error_no);
    }

    return 0;
}


/* sfs_unmount -
 *
 */
static int sfs_unmount(vfs_t * rootfsp)
{
    /* super block 情報の sync とキャッシュ・データの無効化 */
    vnodes_remove(rootfsp->root);
    cache_release(rootfsp->private, false);
    return cache_synchronize(&(rootfsp->device), true);
}


/* sfs_syncfs -
 * 引数 umflag は unmount の時 1，それ以外の場合は 0 にする．
 */
int sfs_syncfs(vfs_t * fsp)
{
    W error_no = cache_synchronize(&(fsp->device), false);
    if (error_no)
	return (error_no);

    return 0;
}

static int sfs_statvfs(vfs_t * fsp, struct statvfs * result)
{
    struct sfs_superblock *sb = (struct sfs_superblock*)(fsp->private);

    result->f_bsize = sb->blksize;
    result->f_frsize = sb->blksize;
    result->f_blocks = sb->nblock;
    result->f_bfree = sb->freeblock;
    result->f_bavail = sb->freeblock;
    result->f_files = sb->ninode;
    result->f_ffree = sb->freeinode;
    result->f_favail = sb->freeinode;
    result->f_fsid = FS_SFS;
    result->f_flag = 0;
    result->f_namemax = SFS_MAXNAMELEN;
    return 0;
}
