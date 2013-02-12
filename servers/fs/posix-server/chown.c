/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: chown.c,v $
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include "fs.h"

W psc_chown_f(RDVNO rdvno, struct posix_request *req)
{
    B path[MAX_NAMELEN];
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;
    W err;

    if (vget_reg(req->caller, req->param.par_chown.path,
		 req->param.par_chown.pathlen + 1, path)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (*path == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	if (proc_get_cwd(req->procid, &startip)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
    }

    if (proc_get_euid(req->procid, &acc.uid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (proc_get_egid(req->procid, &acc.gid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    err = fs_lookup(startip, path, O_RDWR, 0, &acc, &ipp);
    if (err) {
	put_response(rdvno, ENOENT, -1, 0);
	return (FALSE);
    }

    ipp->i_uid = req->param.par_chown.uid;
    ipp->i_gid = req->param.par_chown.gid;
    ipp->i_ctime = get_system_time(NULL);
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
