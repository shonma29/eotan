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

    err = session_get_path(&startip, req->packet.procid,
		 get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		 (UB*)(req->buf));
    if (err)
	return err;

    if (proc_get_permission(req->packet.procid, &acc))
	return EINVAL;

    err = fs_lookup(startip, req->buf, O_RDWR, 0, &acc, &ipp);
    if (err)
	return ENOENT;

    ipp->i_mode = (ipp->i_mode & S_IFMT) | req->packet.args.arg2;
    err = OPS(ipp).wstat(ipp);
    fs_close_file(ipp);
    if (err)
	return err;

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

    error_no = session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

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
