/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: stat.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

/* psc_fstat_f - ファイルの情報を返す
 */
W psc_fstat_f(struct posix_request *req)
{
    struct file *fp;
    W errno;
    struct stat st;

    errno = proc_get_file(req->procid, req->param.par_fstat.fileid, &fp);
    if (errno) {
	put_response(req, errno, -1, 0, 0);
	return (FAIL);
    }

    if (fp == 0) {
	put_response(req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    } else if (fp->f_inode == 0) {
	put_response(req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    st.st_dev = fp->f_inode->i_device;
    st.st_ino = fp->f_inode->i_index;
    st.st_mode = fp->f_inode->i_mode;
    st.st_nlink = fp->f_inode->i_link;
    st.st_size = fp->f_inode->i_size;
    st.st_uid = fp->f_inode->i_uid;
    st.st_gid = fp->f_inode->i_gid;
    st.st_rdev = fp->f_inode->i_dev;
    if (fp->f_inode->i_fs == NULL) {
      printk("FSTAT: illegal i_fs\n");
      st.st_blksize = 512;
    }
    else {
      st.st_blksize = fp->f_inode->i_fs->fs_blksize;
    }
    st.st_blocks = ROUNDUP(st.st_size, st.st_blksize) / st.st_blksize;
    st.st_atime = fp->f_inode->i_atime;
    st.st_mtime = fp->f_inode->i_mtime;
    st.st_ctime = fp->f_inode->i_ctime;

    errno =
	vput_reg(req->caller, req->param.par_fstat.st, sizeof(struct stat),
		 &st);
    if (errno) {
	put_response(req, EP_INVAL, 0, 0, 0);
	return (FAIL);
    }

    put_response(req, EP_OK, 0, 0, 0);

    return (SUCCESS);
}
