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

#include <fcntl.h>
#include <string.h>
#include <utime.h>
#include <core/options.h>
#include <mpu/memory.h>
#include <nerve/kcall.h>
#include <sys/stat.h>
#include "fs.h"
#include "api.h"

int if_chmod(fs_request *req)
{
    vnode_t *startip;
    vnode_t *ipp;
    struct permission acc;
    W err;

    err = session_get_path(&startip, req->packet.procid,
		 get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		 (UB*)(req->buf));
    if (err)
	return err;

    if (proc_get_permission(req->packet.procid, &acc))
	return EINVAL;

    err = vfs_lookup(startip, req->buf, O_RDWR, &acc, &ipp);
    if (err)
	return ENOENT;

    ipp->mode = (ipp->mode & S_IFMT) | req->packet.args.arg2;
    err = ipp->fs->operations.wstat(ipp);
    vnodes_remove(ipp);
    if (err)
	return err;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}

/* if_fstat - ファイルの情報を返す
 */
int if_fstat(fs_request *req)
{
    struct file *fp;
    W error_no;
    struct stat st;

    error_no = session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

    else if (fp->f_inode->fs == NULL)
	return EINVAL;

    fp->f_inode->fs->operations.stat(fp->f_inode, &st);

    error_no =
	kcall->region_put(get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg2), sizeof(struct stat),
		 &st);
    if (error_no)
	return EINVAL;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}
