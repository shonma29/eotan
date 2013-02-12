/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: utime.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include <utime.h>
#include "fs.h"

W psc_utime_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    ER errno = EOK;
    struct utimbuf tb;
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;

    errno = vget_reg(req->caller, req->param.par_utime.path,
		     req->param.par_utime.pathlen + 1, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }

    errno = vget_reg(req->caller, req->param.par_utime.buf,
		     sizeof(struct utimbuf), &tb);
    if (errno) {
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }

    if (*pathname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    put_response(rdvno, errno, -1, 0);
	    return (FALSE);
	}
    } else {
	startip = rootfile;
    }
    errno = proc_get_euid(req->procid, &(acc.uid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = fs_lookup(startip, pathname, O_RDWR, 0, &acc, &ipp);
    if (errno) {
	put_response(rdvno, ENOENT, -1, 0);
	return (FALSE);
    }

    ipp->i_atime = tb.actime;
    ipp->i_mtime = tb.modtime;
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	put_response(rdvno, EINVAL, -1, 0);
	dealloc_inode(ipp);
	return (FALSE);
    }

    dealloc_inode(ipp);

    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}
