/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: getdents.c,v $
 * Revision 1.1  2000/06/01 08:48:06  naniwa
 * first version
 *
 *
 */

#include "fs.h"

W psc_getdents_f(RDVNO rdvno, struct posix_request *req)
{
    W errno;
    struct file *fp;
    W len, flen;

    errno =
	proc_get_file(req->procid, req->param.par_getdents.fileid, &fp);
    if (errno) {
	put_response(rdvno, errno, -1, 0);
	return (FALSE);
    } else if (fp == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    /* 対象ファイルがパイプだったり、
     * ディレクトリ以外の場合には、エラーにする
     */
    if (fp->f_flag & F_PIPE) {
	/* パイプの読み書き */
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if ((fp->f_inode->i_mode & S_IFMT) != S_IFDIR) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    errno = fs_getdents(fp->f_inode, req->caller, fp->f_offset,
			req->param.par_getdents.buf,
			req->param.par_getdents.length, &len, &flen);

    if (errno) {
	put_response(rdvno, errno, -1, 0);
    }

    fp->f_offset += flen;
    put_response(rdvno, EOK, len, 0);
    return (TRUE);
}
