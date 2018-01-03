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
#include "../fs.h"
#include "../vfs.h"
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
sfs_i_create(struct inode * parent,
	     char *fname,
	     W oflag,
	     W mode, struct permission * acc, struct inode ** retip)
{
    struct inode *newip;
    W error_no;
    struct sfs_dir dirent;
    W dirnentry;
    W i_index;
    SYSTIM clock;

    /* 引数のチェック */
    newip = alloc_inode();
    if (newip == NULL) {
	return (ENOMEM);
    }
    *retip = newip;

    /* 新しい sfs_inode をアロケート */
    i_index = sfs_alloc_inode(parent->i_fs->dev.channel, parent->i_fs);
    if (i_index <= 0) {
	return (ENOMEM);
    }

    /* 設定 */
    time_get(&clock);
    newip->i_fs = parent->i_fs;
    newip->i_refcount = 1;
    newip->i_dirty = 1;
    newip->i_mode = mode | S_IFREG;
    newip->i_private.sfs_inode.i_nlink = 1;
    newip->i_index = i_index;
    newip->i_private.sfs_inode.i_uid = acc->uid;
    newip->i_private.sfs_inode.i_gid = acc->gid;
    newip->i_dev = 0;
    newip->i_size = 0;
    newip->i_private.sfs_inode.i_atime = clock;
    newip->i_private.sfs_inode.i_ctime = clock;
    newip->i_private.sfs_inode.i_mtime = clock;
    newip->i_nblock = 0;

    fs_register_inode(newip);

    /* ディレクトリのエントリを作成 */
    dirent.d_index = newip->i_index;
    /* 表示文字長を SFS_MAXNAMELEN にするため．後に pad があるので大丈夫 */
    strncpy(dirent.d_name, fname, SFS_MAXNAMELEN);
    dirent.pad[0] = '\0';

    /* ディレクトリにエントリを追加 */
    dirnentry = sfs_read_dir(parent, 0, NULL);
    error_no = sfs_write_dir(parent, dirnentry, &dirent);
    if (error_no != EOK) {
	return (error_no);
    }

    /* inode 情報の更新 */
    /* 本来は inode の deallocate のところで行う処理のはず */
    error_no = sfs_i_sync(newip);
    if (error_no != EOK) {
	return (error_no);
    }

    return (EOK);
}


/* sfs_i_read -
 *
 */
int sfs_i_read(struct inode * ip, W start, B * buf, W length, W * rlength)
{
    W copysize;
    W offset;
    ID fd;
    struct fs *fsp;
    W bn;
    B *cbuf;

#ifdef FMDEBUG
    dbg_printf
	("sfs: sfs_i_read: start. ip = 0x%x, start = %d, length = %d, buf = 0x%x\n",
	 ip, start, length, buf);
#endif

    fd = ip->i_fs->dev.channel;
    fsp = ip->i_fs;

    if (start + length > ip->i_size) {
	length = ip->i_size - start;
    }
    if (length < 0)
	length = 0;

    *rlength = length;

    while (length > 0) {
#ifdef FMDEBUG
	dbg_printf("sfs: read block: %d\n",
	       sfs_get_block_num(fd, fsp, &(ip->i_private.sfs_inode),
				 start / fsp->fs_blksize));
#endif
	bn = sfs_get_block_num(fd, fsp, &(ip->i_private.sfs_inode),
			       start / fsp->private.sfs_fs.blksize);
	if (bn < 0) {
	    return (EIO);
	}

	cbuf = get_cache(fd, bn);
	offset = start % fsp->private.sfs_fs.blksize;
	if (fsp->private.sfs_fs.blksize - offset < length) {
	    copysize = fsp->private.sfs_fs.blksize - offset;
	} else {
	    copysize = length;
	}

	memcpy(buf, &cbuf[offset], copysize);
	buf += copysize;
	start += copysize;
	length -= copysize;
    }
#ifdef UPDATE_ATIME
    ip->i_atime = get_system_time();
    ip->i_dirty = 1;
#endif
    return (EOK);
}


int sfs_i_write(struct inode * ip, W start, B * buf, W size, W * rsize)
{
    int copysize;
    int offset;
    int retsize;
    int filesize;
    ID fd;
    struct fs *fsp;
    W bn;
    B *cbuf;

#ifdef FMDEBUG
    dbg_printf("sfs: sfs_i_write:(start = %d, size = %d)\n", start, size);	/* */
#endif

    *rsize = 0;
    retsize = size;
    filesize = start + retsize;
    fd = ip->i_fs->dev.channel;
    fsp = ip->i_fs;

    while (size > 0) {
#ifdef FMDEBUG
	dbg_printf("sfs: %s\n",
	       (sfs_get_block_num(fd, fsp, &(ip->i_private.sfs_inode),
				  start / fsp->fs_blksize) <= 0) ?
	       "allocate block" : "read block");
#endif

	if ((bn = sfs_get_block_num(fd, fsp, &(ip->i_private.sfs_inode),
				    start / fsp->private.sfs_fs.blksize)) <= 0) {
	    /* ファイルサイズを越えて書き込む場合には、新しくブロックをアロケートする
	     */
	    bn = sfs_set_block_num(fd, fsp, &(ip->i_private.sfs_inode),
				   start / fsp->private.sfs_fs.blksize,
				   sfs_alloc_block(fd, fsp));
/*
 *   ip->sfs_i_direct[start / fsp->blksize] = alloc_block (fd, fsp);
 */
	    if (bn < 0) {
		return (EIO);
	    }
	    cbuf = get_cache(fd, bn);
	} else {
	    cbuf = get_cache(fd, bn);
	}

	/* 読み込んだブロックの内容を更新する
	 */
	offset = start % fsp->private.sfs_fs.blksize;
	copysize = MIN(fsp->private.sfs_fs.blksize - offset, size);

#ifdef FMDEBUG
	dbg_printf("sfs: *** read block contents ***\n");
	{
	    int i;
	    char tmpbuf[2];

	    tmpbuf[1] = '\0';
	    dbg_printf("sfs: copy size: %d\n", copysize);
	    for (i = 0; i < copysize; i++) {
		tmpbuf[0] = blockbuf[i];
		dbg_printf("sfs: %s", tmpbuf);
	    }
	}
#endif

	memcpy(&cbuf[offset], buf, copysize);

	/* 更新したブロックを書き込む
	 */
	put_cache(cbuf, true);
	buf += copysize;
	start += copysize;
	size -= copysize;
    }

    /* cache の sync をここで行う必要はあるか? */
    sync_cache(fd, 0);

    /* もし、書き込みをおこなった後にファイルのサイズが増えていれば、
     * サイズを更新して inode を書き込む。
     */
    if (filesize > ip->i_size) {
        SYSTIM clock;

        time_get(&clock);
	ip->i_size = filesize;
	ip->i_nblock =
	    roundUp(filesize, fsp->private.sfs_fs.blksize) / fsp->private.sfs_fs.blksize;
	ip->i_private.sfs_inode.i_mtime = clock;
	ip->i_private.sfs_inode.i_ctime = clock;
	ip->i_dirty = 1;
	/* これは deallocate の中で処理するのが普通 */
	sfs_i_sync(ip);
    }

    *rsize = retsize - size;

#ifdef FMDEBUG
    dbg_printf("sfs: write size: %d bytes\n", *rsize);
#endif

    return (EOK);
}


W sfs_i_truncate(struct inode * ip, W newsize)
{
    int nblock, blockno, inblock, offset;
    W error_no;
    W fd;
    struct fs *fsp;
    struct sfs_inode *sfs_ip;
    SYSTIM clock;

    fd = ip->i_fs->dev.channel;
    fsp = ip->i_fs;
    sfs_ip = &(ip->i_private.sfs_inode);
    nblock = roundUp(newsize, fsp->private.sfs_fs.blksize) / fsp->private.sfs_fs.blksize;
    if (nblock < sfs_ip->i_nblock) {
	/* 余分なブロックを開放 */
	blockno = nblock;
	if (blockno < (SFS_INDIRECT_BLOCK_ENTRY * SFS_INDIRECT_BLOCK)) {
	    /* 一重間接ブロックの範囲内 */
	    inblock = blockno;
	    offset = inblock % SFS_INDIRECT_BLOCK;
	    inblock = inblock / SFS_INDIRECT_BLOCK;
	    sfs_free_indirect(fd, fsp, sfs_ip, offset, inblock);
	}
    }

    ip->i_size = newsize;
    ip->i_nblock = nblock;
    time_get(&clock);
    ip->i_private.sfs_inode.i_mtime = clock;
    ip->i_private.sfs_inode.i_ctime = clock;
    ip->i_dirty = 1;

    /* ここで fs を sync する必要があるか? */
    error_no = sfs_syncfs(fsp, 0);
    if (error_no) {
	return (error_no);
    }
    return (EOK);
}
