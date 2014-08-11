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
#include "fs.h"

void psc_chmod_f(RDVNO rdvno, struct posix_request *req)
{
    B path[MAX_NAMELEN];
    struct inode *startip;
    struct inode *ipp;
    struct permission acc;
    W err;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    if (kcall->region_get(get_rdv_tid(rdvno), req->param.par_chmod.path,
		 req->param.par_chmod.pathlen + 1, path)) {
	put_response(rdvno, EINVAL, -1, 0);
	return;
    }

    if (*path == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	if (proc_get_cwd(req->procid, &startip)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return;
	}
    }

    if (proc_get_permission(req->procid, &acc)) {
	put_response(rdvno, EINVAL, -1, 0);
	return;
    }

    err = fs_lookup(startip, path, O_RDWR, 0, &acc, &ipp);
    if (err) {
	put_response(rdvno, ENOENT, -1, 0);
	return;
    }

    ipp->i_mode = (ipp->i_mode & S_IFMT) | req->param.par_chmod.mode;
    OPS(ipp).wstat(ipp);
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	put_response(rdvno, EINVAL, -1, 0);
	dealloc_inode(ipp);
	return;
    }

    dealloc_inode(ipp);
    put_response(rdvno, EOK, 0, 0);
}

/* psc_fstat_f - ファイルの情報を返す
 */
void psc_fstat_f(RDVNO rdvno, struct posix_request *req)
{
    struct file *fp;
    W error_no;
    struct stat st;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = proc_get_file(req->procid, req->param.par_fstat.fileid, &fp);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
	return;
    }

    if (fp == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return;
    } else if (fp->f_inode == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return;
    } else if (fp->f_inode->i_fs == NULL) {
	put_response(rdvno, EINVAL, -1, 0);
	return;
    }

    fp->f_inode->i_fs->ops.stat(fp->f_inode, &st);

    error_no =
	kcall->region_put(get_rdv_tid(rdvno), req->param.par_fstat.st, sizeof(struct stat),
		 &st);
    if (error_no) {
	put_response(rdvno, EINVAL, 0, 0);
	return;
    }

    put_response(rdvno, EOK, 0, 0);
}

void
psc_statvfs_f (RDVNO rdvno, struct posix_request *req)
{
  struct statvfs	result;
  ER		error_no;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  error_no = fs_statvfs (req->param.par_statvfs.device, &result);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);    
      return;
    }

  error_no = kcall->region_put(get_rdv_tid(rdvno), req->param.par_statvfs.fsp, sizeof (struct statvfs), &result);
  if (error_no)
    {
      put_response (rdvno, EFAULT, -1, 0);
      return;
    }
  put_response (rdvno, EOK, 0, 0);
}
