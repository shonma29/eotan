/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: umount.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W psc_umount_f(struct posix_request *req)
{
    W errno;
    UW device = 0;
    B dirname[MAX_NAMELEN];
    struct inode *startip;
    struct inode *umpoint;
    struct access_info acc;

    errno = vget_reg(req->caller,
		     req->param.par_umount.dirname,
		     req->param.par_umount.dirnamelen + 1, dirname);

    if (errno) {
	/* mount 先/special file のパス名のコピーエラー */
	if (errno == E_PAR) {
	    put_response(req, EP_INVAL, -1, 0, 0);
	} else {
	    put_response(req, EP_FAULT, -1, 0, 0);
	}
	return (FAIL);
    }

    /* アンマウントポイントのオープン */
    if (*dirname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    put_response(req, errno, -1, 0, 0);
	    return (FAIL);
	}
    } else {
	startip = rootfile;
    }

    errno = proc_get_euid(req->procid, &(acc.uid));
    if (errno) {
	put_response(req, errno, -1, 0, 0);
	return (FAIL);
    }
    if (acc.uid != SU_UID) {
	put_response(req, EP_ACCESS, -1, 0, 0);
	return (FAIL);
    }

    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(req, errno, -1, 0, 0);
	return (FAIL);
    }

    errno = fs_open_file(dirname, O_RDWR, 0, &acc, startip, &umpoint);
    if (errno) {
	put_response(req, errno, -1, 0, 0);
	return (FAIL);
    }

    errno = EP_OK;
    switch (umpoint->i_mode & FS_FMT_MSK) {
    case FS_FMT_DIR:
	device = umpoint->i_device;
	break;
    case FS_FMT_BDEV:
	device = umpoint->i_dev;
	break;
    case FS_FMT_REG:
        errno = EP_NOTDIR;
	break;
      default:
	errno = EP_INVAL;
	break;
    }

    fs_close_file(umpoint);
    if (errno == EP_OK) {
	errno = umount_fs(device);
    }
    if (errno) {
#if 0
        dbg_printf("[PM] umount errno = %d\n", errno);
#endif
	put_response(req, errno, -1, 0, 0);
	return (FAIL);
    }
    put_response(req, EP_OK, 0, 0, 0);
    return (SUCCESS);
}
