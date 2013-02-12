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

#include <fcntl.h>
#include "fs.h"

W psc_umount_f(RDVNO rdvno, struct posix_request *req)
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
	    put_response(rdvno, EINVAL, -1, 0);
	} else {
	    put_response(rdvno, EFAULT, -1, 0);
	}
	return (FALSE);
    }

    /* アンマウントポイントのオープン */
    if (*dirname != '/') {
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
    if (acc.uid != SU_UID) {
	put_response(rdvno, EACCESS, -1, 0);
	return (FALSE);
    }

    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = fs_open_file(dirname, O_RDWR, 0, &acc, startip, &umpoint);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = EOK;
    switch (umpoint->i_mode & S_IFMT) {
    case S_IFDIR:
	device = umpoint->i_device;
	break;
    case S_IFBLK:
	device = umpoint->i_dev;
	break;
    case S_IFREG:
        errno = ENOTDIR;
	break;
      default:
	errno = EINVAL;
	break;
    }

    fs_close_file(umpoint);
    if (errno == EOK) {
	errno = umount_fs(device);
    }
    if (errno) {
#if 0
        dbg_printf("[PM] umount errno = %d\n", errno);
#endif
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}
