/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* sfs_dir.c - SFS のディレクトリに関係する処理を行う。
 *
 * $Log: sfs_dir.c,v $
 * Revision 1.11  2000/06/01 08:47:24  naniwa
 * to implement getdents
 *
 * Revision 1.10  1999/05/30 04:05:32  naniwa
 * modified to creat file correctly
 *
 * Revision 1.9  1999/05/28 15:48:34  naniwa
 * sfs ver 1.1
 *
 * Revision 1.8  1999/05/10 15:46:16  night
 * ディレクトリ内容の書き込みをする処理で、sfs_i_read () を呼びだしていた
 * のを sfs_i_write () を呼び出すように変更。
 *
 * Revision 1.7  1999/04/13 04:15:26  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.6  1999/03/24 04:52:16  monaka
 * Source file cleaning for avoid warnings.
 *
 * Revision 1.5  1997/07/04 15:07:41  night
 * ・スペシャルファイル - デバイスドライバポートの対応表の関連処理の追加。
 * ・ファイルの読み込み処理の改訂。
 *
 * Revision 1.4  1997/07/03 14:24:35  night
 * mountroot/open 処理のバグを修正。
 *
 * Revision 1.3  1996/11/27 15:42:08  night
 * sfs_write_dir() を追加
 *
 * Revision 1.2  1996/11/18 13:46:10  night
 * ファイルの中身を作成。
 *
 * Revision 1.1  1996/11/17  14:52:57  night
 * 最初の登録
 *
 *
 */

#include <fcntl.h>
#include <string.h>
#include <fs/sfs.h>
#include <nerve/kcall.h>
#include <sys/dirent.h>
#include <sys/errno.h>
#include "../libserv/libserv.h"
#include "func.h"



int sfs_i_link(vnode_t * parent, char *fname, vnode_t * srcip)
{
    W error_no;

    error_no = tfs_append_entry(parent, fname, srcip);
    if (error_no) {
	return (error_no);
    }

    /* inode 情報の更新 */
    struct sfs_inode *sfs_inode = srcip->private;
    srcip->nlink += 1;
    time_get(&(sfs_inode->i_ctime));
    srcip->dirty = true;

    return 0;
}


int
sfs_i_unlink(vnode_t * parent, char *fname, vnode_t *ip)
{
    W error_no;

    /* ファイルの名前の最後の１つで，使用中なら削除しない */
    if ((ip->nlink == 1) && (ip->refer_count >= 2)) {
	return (EBUSY);
    }

    error_no = tfs_remove_entry(parent, fname, ip);
    if (error_no) {
	return (error_no);
    }

    if (ip->nlink <= 0) {
	sfs_i_truncate(ip, 0);
	sfs_free_inode(ip->fs, ip);
    }
    return 0;
}


/*
 * ディレクトリを削除する。
 *
 */
int sfs_i_rmdir(vnode_t * parent, char *fname, vnode_t *ip)
{
    int nentry;
    W error_no;

    nentry = ip->size / sizeof(struct sfs_dir);
    if (nentry >= 3) {
	return (ENOTEMPTY);
    }

    error_no = tfs_remove_entry(parent, fname, ip);
    if (error_no) {
	return (error_no);
    }

    struct sfs_inode *sfs_inode = ip->private;
    if (ip->nlink <= 1) {
	sfs_i_truncate(ip, 0);
	sfs_free_inode(ip->fs, ip);
    }
    sfs_inode = parent->private;
    parent->nlink -= 1;
    time_get(&(sfs_inode->i_ctime));
    parent->dirty = true;

    return 0;
}
