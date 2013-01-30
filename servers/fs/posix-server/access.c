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

#include <string.h>
#include "fs.h"

W psc_access_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    W fileid;
    W errno;
    struct inode *startip;
    struct inode *newip;
    struct access_info acc;
    W euid;
    W accmode;



    errno = proc_alloc_fileid(req->procid, &fileid);
    if (errno) {
	/* メモリ取得エラー */
	put_response(rdvno, EP_NOMEM, -1, 0);
	return (FAIL);
    }
    memset(pathname, 0, req->param.par_access.pathlen + 1);

    /* パス名をユーザプロセスから POSIX サーバにコピーする。
     */
    errno =
	vget_reg(req->caller, req->param.par_access.path,
		 req->param.par_access.pathlen, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EP_INVAL, -1, 0);
	else
	    put_response(rdvno, EP_FAULT, -1, 0);

	return (FAIL);
    }
#ifdef notdef
    printf("psc_access_f: open file path = %s\n", pathname);
#endif
    if (*pathname != '/') {
	errno = proc_get_cwd(req->procid, &startip);
	if (errno) {
	    put_response(rdvno, errno, -1, 0);
	    return (FAIL);
	}
    } else {
	startip = rootfile;
    }
    errno = proc_get_uid(req->procid, &(acc.uid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FAIL);
    }
    errno = proc_get_euid(req->procid, &euid);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FAIL);
    }

    errno = proc_get_gid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FAIL);
    }

    errno = fs_open_file(pathname,
			 req->param.par_open.oflag,
			 req->param.par_open.mode, &acc, startip, &newip);
    if (errno) {
#ifdef notdef
	printf("open systemcall: Not found entry.\n");
#endif
	/* ファイルがオープンできない */
	put_response(rdvno, errno, -1, 0);
	return (FAIL);
    }

    accmode = req->param.par_access.accflag;
    if (newip->i_uid == acc.uid) {
	accmode |= (req->param.par_access.accflag << 6);
    }
    if (newip->i_gid == acc.gid) {
	accmode |= (req->param.par_access.accflag << 3);
    }

    errno = fs_close_file(newip);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FAIL);
    }

    /* アクセス権限のチェック */
    if ((acc.uid == 0) || (euid == 0)) {
	/* root ユーザの場合には、無条件で成功とする */
	put_response(rdvno, EP_OK, 0, 0);
	return (SUCCESS);
    } else if ((newip->i_mode & accmode) == 0) {
	put_response(rdvno, EP_ACCESS, -1, 0);
	return (FAIL);
    }

    put_response(rdvno, EP_OK, 0, 0);
    return (SUCCESS);
}
