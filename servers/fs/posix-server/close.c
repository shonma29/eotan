/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: close.c,v $
 * Revision 1.3  2000/02/04 15:16:25  naniwa
 * minor fix
 *
 * Revision 1.2  1999/05/30 04:08:19  naniwa
 * modified to clear closed file slot
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W psc_close_f(RDVNO rdvno, struct posix_request *req)
{
    struct file *fp;
    W err;
    ER sfs_close_device();

    err = proc_get_file(req->procid, req->param.par_close.fileid, &fp);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    if (fp->f_inode == NULL) {
	put_response(rdvno, EBADF, -1, 0);
	return (FALSE);
    }

    if (fp->f_inode->i_mode & S_IFCHR) {
	/* スペシャルファイルだった */
	/* デバイスに DEV_CLS メッセージを発信 */
	err = sfs_close_device(fp->f_inode->i_dev);
    }

    err = fs_close_file(fp->f_inode);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    fp->f_inode = NULL;
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}
