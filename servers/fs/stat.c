/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: access.c,v $
 * Revision 1.3  2000/05/06 03:54:22  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.2  1999/07/10 10:45:34  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include <string.h>
#include <utime.h>
#include <core/options.h>
#include <mpu/memory.h>
#include <nerve/kcall.h>
#include <sys/stat.h>
#include "fs.h"
#include "api.h"

int if_chmod(fs_request *req)
{
    struct inode *startip;
    struct inode *ipp;
    struct permission acc;
    W err;

    if (kcall->region_copy(get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		 sizeof(req->buf) - 1, req->buf) < 0)
	return EINVAL;

    req->buf[MAX_NAMELEN] = '\0';

    if (req->buf[0] == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	if (proc_get_cwd(req->packet.procid, &startip))
	    return EINVAL;
    }

    if (proc_get_permission(req->packet.procid, &acc))
	return EINVAL;

    err = fs_lookup(startip, req->buf, O_RDWR, 0, &acc, &ipp);
    if (err)
	return ENOENT;

    ipp->i_mode = (ipp->i_mode & S_IFMT) | req->packet.args.arg2;
    OPS(ipp).wstat(ipp);
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	dealloc_inode(ipp);
	return EINVAL;
    }

    dealloc_inode(ipp);
    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}

/* if_fstat - ファイルの情報を返す
 */
int if_fstat(fs_request *req)
{
    struct file *fp;
    W error_no;
    struct stat st;

    error_no = proc_get_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

    if (fp == 0)
	return EINVAL;

    else if (fp->f_inode == 0)
	return EINVAL;

    else if (fp->f_inode->i_fs == NULL)
	return EINVAL;

    fp->f_inode->i_fs->ops.stat(fp->f_inode, &st);

    error_no =
	kcall->region_put(get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg2), sizeof(struct stat),
		 &st);
    if (error_no)
	return EINVAL;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}

int
if_statvfs (fs_request *req)
{
  struct statvfs	result;
  ER		error_no;

  error_no = fs_statvfs (req->packet.param.par_statvfs.device, &result);
  if (error_no)
      return error_no;

  error_no = kcall->region_put(get_rdv_tid(req->rdvno), req->packet.param.par_statvfs.fsp, sizeof (struct statvfs), &result);
  if (error_no)
      return EFAULT;

  put_response (req->rdvno, EOK, 0, 0);
  return EOK;
}
