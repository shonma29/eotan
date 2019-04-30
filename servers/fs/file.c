/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: close.c,v $
 * Revision 1.3  2000/02/04 15:16:25  naniwa
 * minor fix
 *
 * Revision 1.2  1999/05/30 04:08:19  naniwa
 * modified to clear closed file slot
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <device.h>
#include <fcntl.h>
#include <string.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "api.h"
#include "fs.h"
#include "procfs/process.h"

int if_close(fs_request *req)
{
    struct file *fp;
    W err;

    req->packet.process_id &= 0xffff;
    err = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (err)
	return err;

    err = vnodes_remove(fp->f_vnode);
    if (err)
	return err;

    fp->f_vnode = NULL;
    reply2(req->rdvno, 0, 0, 0);
    return EOK;
}

int if_lseek(fs_request *req)
{
    struct file *fp;
    W error_no;
    off_t *offp = (off_t*)&(req->packet.arg2);

    req->packet.process_id &= 0xffff;
    error_no = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (error_no)
	return error_no;

    switch (req->packet.arg4) {
    case SEEK_SET:
	fp->f_offset = *offp;
	break;

    case SEEK_CUR:
	fp->f_offset += *offp;
	break;

    case SEEK_END:
	fp->f_offset = fp->f_vnode->size + *offp;
	break;

    default:
	return EINVAL;
    }

    if (fp->f_offset < 0) {
	fp->f_offset = 0;
    }
    else if (fp->f_vnode->mode & S_IFCHR) {
      if (fp->f_offset > fp->f_vnode->size) {
	/* ブロックデバイスなど，サイズの制限のあるデバイスの場合 */
	fp->f_offset = fp->f_vnode->size;
      }
    }

    reply64(req->rdvno, EOK, fp->f_offset);
    return EOK;
}

/* if_open - ファイルのオープン
 */
int if_open(fs_request *req)
{
    int fileid;
    W error_no;
    vnode_t *startip;
    vnode_t *newip;
    struct permission acc;
    ID caller = (req->packet.process_id >> 16) & 0xffff;

    req->packet.process_id &= 0xffff;
    error_no = proc_alloc_fileid(req->packet.process_id, &fileid);
    if (error_no)
	/* メモリ取得エラー */
	return ENOMEM;

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = session_get_path(&startip, req->packet.process_id,
		    caller, (UB*)(req->packet.arg1),
		    (UB*)(req->buf));
    if (error_no)
	return error_no;

    error_no = proc_get_permission(req->packet.process_id, &acc);
    if (error_no)
	return error_no;

    error_no = vfs_open(startip, req->buf,
			 req->packet.arg2,
			 req->packet.arg3,
			 &acc, &newip);
    if (error_no)
	/* ファイルがオープンできない */
	return error_no;

    if ((newip->mode & S_IFMT) == S_IFDIR) {
	/* ファイルは、ディレクトリだった
	 * エラーとする
	 */
	if (req->packet.arg2 != O_RDONLY) {
	    vnodes_remove(newip);
	    return EISDIR;
	}
    }

    if (proc_set_file(req->packet.process_id, fileid,
		      req->packet.arg2, newip)) {
	vnodes_remove(newip);
	return EINVAL;
    }

    reply2(req->rdvno, 0, fileid, 0);
    return EOK;
}

/* if_read - ファイルからのデータの読み込み
 */
int if_read(fs_request *req)
{
    W error_no;
    struct file *fp;
    size_t rlength;
    W rest_length;
    W i, len;
    ID caller = (req->packet.process_id >> 16) & 0xffff;

    req->packet.process_id &= 0xffff;
    error_no = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (error_no)
	return error_no;

    if (fp->f_flag == O_WRONLY)
	return EBADF;

    int offset = fp->f_offset;
    for (i = 0, rest_length = req->packet.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	/* MAX_BODY_SIZE 毎にファイルに読み込み */
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	int delta = fs_read(fp->f_vnode, req->buf, offset, len, &rlength);
	if (delta < 0) {
	    return (-delta);
	} else if (!rlength)
	    break;

	/* 呼び出したプロセスのバッファへの書き込み */
	error_no = kcall->region_put(caller, (UB*)(req->packet.arg2) + i,
			 rlength, req->buf);
	if (error_no)
	    return EFAULT;

	offset += delta;
    }

    fp->f_offset = offset;
    reply2(req->rdvno, 0, i, 0);
    return EOK;
}

int if_write(fs_request *req)
{
    W error_no;
    struct file *fp;
    size_t rlength;
    W i, len;
    W rest_length;
    ID caller = (req->packet.process_id >> 16) & 0xffff;

    req->packet.process_id &= 0xffff;
    error_no = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (error_no)
	return error_no;

    if (fp->f_flag == O_RDONLY)
	return EBADF;

    if ((! (fp->f_vnode->mode & S_IFCHR)) &&
	(fp->f_offset > fp->f_vnode->size)) {
      /* 通常ファイルで，書き込む場所がファイルの内容が存在しない場所 */
      /* そこまでを 0 で埋める */
      memset(req->buf, 0, sizeof(req->buf));
      for (rest_length = fp->f_offset - fp->f_vnode->size;
	   rest_length > 0; rest_length -= rlength) {
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no = vfs_write(fp->f_vnode, req->buf,
			      fp->f_vnode->size, len, &rlength);
	if (error_no || (rlength < len)) {
	  break;
	}
      }
    }
    if (error_no)
	return error_no;

    for (i = 0, rest_length = req->packet.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no =
	    kcall->region_get(caller, (UB*)(req->packet.arg2) + i, len, req->buf);
	if (error_no)
	    break;

	error_no = vfs_write(fp->f_vnode, req->buf,
			      fp->f_offset + i, len, &rlength);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }

    if (error_no)
	return error_no;

    fp->f_offset += i;
    reply2(req->rdvno, 0, i, 0);
    return EOK;
}
