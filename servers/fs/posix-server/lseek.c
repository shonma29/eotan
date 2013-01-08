/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: lseek.c,v $
 * Revision 1.3  2000/02/04 15:15:10  naniwa
 * corrected log comment
 *
 * Revision 1.2  2000/02/01 08:12:30  naniwa
 * In this time, lseek can not move beyond EOF
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W psc_lseek_f(RDVNO rdvno, struct posix_request *req)
{
    struct file *fp;
    W errno;

    errno = proc_get_file(req->procid, req->param.par_lseek.fileid, &fp);
    if (errno) {
	put_response(rdvno, req, errno, -1, errno, 0);
	return (FAIL);
    }

    switch (req->param.par_lseek.mode) {
    case SEEK_SET:
	fp->f_offset = req->param.par_lseek.offset;
	break;

    case SEEK_CUR:
	fp->f_offset += req->param.par_lseek.offset;
	break;

    case SEEK_END:
	fp->f_offset = fp->f_inode->i_size + req->param.par_lseek.offset;
	break;

    default:
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    if (fp->f_offset < 0) {
	fp->f_offset = 0;
    }
    else if (fp->f_inode->i_mode & FS_FMT_DEV) {
      if (fp->f_offset > fp->f_inode->i_size) {
	/* ブロックデバイスなど，サイズの制限のあるデバイスの場合 */
	fp->f_offset = fp->f_inode->i_size;
      }
    }
#ifdef notdef 
    else if (fp->f_offset > fp->f_inode->i_size) {
	/* デバイスドライバでない通常のファイルの場合 */
	/* EOF を越えた読み書きはまだサポートされていない */
	fp->f_offset = fp->f_inode->i_size;
    }
#endif

    put_response(rdvno, req, EP_OK, fp->f_offset, 0, 0);

    return (SUCCESS);
}
