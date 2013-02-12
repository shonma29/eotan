/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: link.c,v $
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W psc_link_f(RDVNO rdvno, struct posix_request *req)
{
    B src[MAX_NAMELEN], dst[MAX_NAMELEN];
    struct access_info acc;
    W errno;

    errno = vget_reg(req->caller, req->param.par_link.src,
		     req->param.par_link.srclen + 1, src);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }
    errno = vget_reg(req->caller, req->param.par_link.dst,
		     req->param.par_link.dstlen + 1, dst);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }

    /* プロセスのユーザ ID とグループ ID の
     * 取り出し。
     * この情報に基づいて、ファイルを削除できるかどうかを
     * 決定する。
     */
    errno = proc_get_euid(req->procid, &(acc.uid));
    if (errno) {
	put_response(rdvno, errno, 0, 0);
	return (FALSE);
    }
    errno = proc_get_egid(req->procid, &(acc.gid));
    if (errno) {
	put_response(rdvno, errno, 0, 0);
	return (FALSE);
    }

    errno = fs_link_file(req->procid, src, req->param.par_link.srclen,
			 dst, req->param.par_link.dstlen, &acc);
    if (errno) {
	put_response(rdvno, errno, 0, 0);
	return (FALSE);
    }
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}
