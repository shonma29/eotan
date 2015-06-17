/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: chdir.c,v $
 * Revision 1.3  2000/05/20 12:39:21  naniwa
 * to check as a directry
 *
 * Revision 1.2  2000/05/20 11:57:17  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include "fs.h"
#include "api.h"

void if_chdir(fs_request *req)
{
    struct inode *oldip;
    struct inode *startip;
    struct inode *ipp;
    struct permission acc;
    W err;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    if (kcall->region_copy(get_rdv_tid(req->rdvno), req->packet.param.par_chdir.path,
		sizeof(req->buf) - 1, req->buf) < 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    req->buf[MAX_NAMELEN] = '\0';
    err = proc_get_cwd(req->packet.procid, &oldip);
    if (err) {
	put_response(req->rdvno, err, -1, 0);
	return;
    }

    if (req->buf[0] == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	startip = oldip;
    }


    if (proc_get_permission(req->packet.procid, &acc)) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    err = fs_lookup(startip, req->buf, O_RDONLY, 0, &acc, &ipp);
    if (err) {
	put_response(req->rdvno, err, -1, 0);
	return;
    }

    if ((ipp->i_mode & S_IFMT) != S_IFDIR) {
	/* ファイルは、ディレクトリではなかった。
	 * エラーとする
	 * 
	 */
	fs_close_file(ipp);
	put_response(req->rdvno, ENOTDIR, -1, 0);
	return;
    }

    err = OPS(ipp).permit(ipp, &acc, X_OK);
    if (err) {
	put_response(req->rdvno, err, -1, 0);
	return;
    }

    err = proc_set_cwd(req->packet.procid, ipp);
    if (err) {
	put_response(req->rdvno, err, -1, 0);
	return;
    }

    dealloc_inode(oldip);
    put_response(req->rdvno, EOK, 0, 0);
}
