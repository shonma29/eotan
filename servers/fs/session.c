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
#include <sys/stat.h>
#include "fs.h"
#include "api.h"

int if_chdir(fs_request *req)
{
    vnode_t *oldip;
    vnode_t *startip;
    vnode_t *ipp;
    struct permission acc;
    W err;

    if (kcall->region_copy(get_rdv_tid(req->rdvno), req->packet.param.par_chdir.path,
		sizeof(req->buf) - 1, req->buf) < 0)
	return EINVAL;

    req->buf[MAX_NAMELEN] = '\0';
    err = proc_get_cwd(req->packet.procid, &oldip);
    if (err)
	return err;

    if (req->buf[0] == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
	startip = oldip;
    }


    if (proc_get_permission(req->packet.procid, &acc))
	return EINVAL;

    err = fs_lookup(startip, req->buf, O_RDONLY, 0, &acc, &ipp);
    if (err)
	return err;

    if ((ipp->mode & S_IFMT) != S_IFDIR) {
	/* ファイルは、ディレクトリではなかった。
	 * エラーとする
	 * 
	 */
	vnodes_remove(ipp);
	return ENOTDIR;
    }

    err = ipp->fs->operations.permit(ipp, &acc, X_OK);
    if (err)
	return err;

    err = proc_set_cwd(req->packet.procid, ipp);
    if (err)
	return err;

    vnodes_remove(oldip);
    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}

W session_get_opened_file(const ID pid, const W fno, struct file **fp)
{
    W err = proc_get_file(pid, fno, fp);

    if (err)
	return err;

    if ((*fp)->f_inode == NULL)
	return EBADF;

    return EOK;
}

W session_get_path(vnode_t **ip, const ID pid, const ID tid,
	UB *src, UB *buf)
{
    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    W error_no = kcall->region_copy(tid, src, MAX_NAMELEN, buf);

    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    buf[MAX_NAMELEN] = '\0';

    if (buf[0] != '/') {
	error_no = proc_get_cwd(pid, ip);
	if (error_no)
	    return error_no;

    } else
	*ip = rootfile;

    return EOK;
}
