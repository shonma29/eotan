/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: chmod.c,v $
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W psc_chmod_f(RDVNO rdvno, struct posix_request *req)
{
#ifdef USE_ALLOCA
    B *path;
#else
    B path[MAX_NAMELEN];
#endif
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;
    W err;

#ifdef USE_ALLOCA
    path = alloca(req->param.par_chmod.pathlen + 1);
    if (path == 0) {
	put_response(rdvno, req, EP_NOMEM, -1, 0, 0);
	return (FAIL);
    }
#endif

    if (vget_reg(req->caller, req->param.par_chmod.path,
		 req->param.par_chmod.pathlen + 1, path)) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    if (*path == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	if (proc_get_cwd(req->procid, &startip)) {
	    put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	    return (FAIL);
	}
    }

    if (proc_get_euid(req->procid, &acc.uid)) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    if (proc_get_egid(req->procid, &acc.gid)) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }
    err = fs_lookup(startip, path, O_RDWR, 0, &acc, &ipp);
    if (err) {
	put_response(rdvno, req, EP_NOENT, -1, 0, 0);
	return (FAIL);
    }

    ipp->i_mode = (ipp->i_mode & FS_FMT_MSK) | req->param.par_chmod.mode;
    ipp->i_ctime = get_system_time(NULL);
    ipp->i_dirty = 1;

    /* fs_close_file で行う処理 */
    if (fs_sync_file(ipp)) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	dealloc_inode(ipp);
	return (FAIL);
    }

    dealloc_inode(ipp);
    put_response(rdvno, req, EP_OK, 0, 0, 0);
    return (SUCCESS);
}
