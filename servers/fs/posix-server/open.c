/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: open.c,v $
 * Revision 1.5  2000/06/23 09:18:38  naniwa
 * to support O_APPEND
 *
 * Revision 1.4  2000/05/20 11:57:16  naniwa
 * minor fix
 *
 * Revision 1.3  2000/05/06 03:54:27  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.2  2000/02/04 15:16:31  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include "fs.h"

extern W sfs_open_device(ID device, W * rsize);

/* psc_open_f - ファイルのオープン
 */
W psc_open_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    W fileid;
    W errno;
    struct inode *startip;
    struct inode *newip;
    struct access_info acc;
    W umask;
    W rsize;

    errno = proc_alloc_fileid(req->procid, &fileid);
    if (errno) {
	/* メモリ取得エラー */
	put_response(rdvno, ENOMEM, -1, 0);
	return (FALSE);
    }

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    errno = vget_reg(req->caller, req->param.par_open.path,
		     req->param.par_open.pathlen + 1, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }
#ifdef notdef
    dbg_printf("psc_open_f: open file path len = %d [%s]\n",
	       req->param.par_open.pathlen, pathname);
#endif
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

    errno = proc_get_umask(req->procid, &umask);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    errno = fs_open_file(pathname,
			 req->param.par_open.oflag,
			 req->param.par_open.mode & (~umask),
			 &acc, startip, &newip);
    if (errno) {
#ifdef notdef
	printf("open systemcall: Not found entry.\n");
#endif
	/* ファイルがオープンできない */
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    }

    if ((newip->i_mode & S_IFMT) == S_IFDIR) {
#ifdef notdef
	W uid, euid;
#endif

	/* ファイルは、ディレクトリだった
	 * エラーとする
	 */

	/* root ユーザの場合には、
	 * 成功でもよい
	 */
#ifdef notdef
	if (proc_get_uid(req->procid, &uid)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
	if (proc_get_euid(req->procid, &euid)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
	if ((uid != 0) && (euid != 0)) {
	    fs_close_file(newip);
	    put_response(rdvno, EISDIR, -1, 0);
	    return (FALSE);
	}
#else
	if (acc.uid != SU_UID) {
	    fs_close_file(newip);
	    put_response(rdvno, EACCESS, -1, 0);
	    return (FALSE);
	}
#endif
	if (req->param.par_open.oflag != O_RDONLY) {
	    fs_close_file(newip);
	    put_response(rdvno, EISDIR, -1, 0);
	    return (FALSE);
	}
    } else if (newip->i_mode & S_IFCHR) {
	/* スペシャルファイルだった */
	/* デバイスに DEV_OPN メッセージを発信 */
	errno = sfs_open_device(newip->i_dev, &rsize);
	if (rsize >= 0) {
	    newip->i_size = rsize;
	}
	if (errno != E_OK) {
	    fs_close_file(newip);
	    put_response(rdvno, EACCESS, -1, 0);
	    return (FALSE);
	}
    }

    if (proc_set_file(req->procid, fileid,
		      req->param.par_open.oflag, newip)) {
	fs_close_file(newip);
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    put_response(rdvno, EOK, fileid, 0);
    return (TRUE);
}
