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
    session_t *session = session_find(req->packet.process_id);
    if (!session)
	return ESRCH;

    fp = session_find_desc(session, req->packet.arg1);
    if (!fp)
	return EBADF;

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
    W error_no = 0;
    struct file *fp;
    size_t rlength;
    W i, len;
    W rest_length;
    ID caller = (req->packet.process_id >> 16) & 0xffff;

    req->packet.process_id &= 0xffff;
    session_t *session = session_find(req->packet.process_id);
    if (!session)
	return ESRCH;

    fp = session_find_desc(session, req->packet.arg1);
    if (!fp)
	return EBADF;

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
