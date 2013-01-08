/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/*
 * $Log: read.c,v $
 * Revision 1.8  2000/05/07 10:24:49  naniwa
 * minor fix
 *
 * Revision 1.7  2000/05/06 03:54:30  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.6  2000/02/27 15:35:46  naniwa
 * to work as multi task OS
 *
 * Revision 1.5  2000/02/20 09:39:40  naniwa
 * to use MAX_BODY_SIZE
 *
 * Revision 1.4  2000/02/04 15:16:33  naniwa
 * minor fix
 *
 * Revision 1.3  2000/01/21 14:26:47  naniwa
 * fixed argument of fs_read_file
 *
 * Revision 1.2  2000/01/15 15:30:53  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

extern W sfs_read_device(ID device, B * buf, W start, W length, W * rlength);


/* psc_read_f - ファイルからのデータの読み込み
 */
W psc_read_f(RDVNO rdvno, struct posix_request *req)
{
    W errno;
    struct file *fp;
    W rlength;
    W rest_length;
    W i, len;
    static B buf[MAX_BODY_SIZE];

    errno = proc_get_file(req->procid, req->param.par_read.fileid, &fp);
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	return (FAIL);
    } else if (fp == 0) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    } else if (fp->f_inode == 0) {
	put_response(rdvno, req, EP_INVAL, -1, 0, 0);
	return (FAIL);
    }

    if (fp->f_omode == O_WRONLY) {
	put_response(rdvno, req, EP_BADF, -1, 0, 0);
	return (FAIL);
    }

    if (fp->f_flag & F_PIPE) {
	/* パイプの読み書き */
	put_response(rdvno, req, EP_NOSUP, -1, 0, 0);
	return (FAIL);
    }

    if (fp->f_inode->i_mode & FS_FMT_DEV) {
	if ((fp->f_inode->i_dev & BLOCK_DEVICE_MASK) == 0) {
	    /* スペシャルファイル（キャラクタデバイス）だった */
	    if (req->param.par_read.length > 1) {
		/* データの読み込みを 1 byte ずつに分割する */
		req->param.par_read.length = 1;
	    }
	} else {
	    /* ブロックデバイスだった */
	    if (fp->f_offset >= fp->f_inode->i_size) {
		errno = EP_OK;
		req->param.par_read.length = 0;
	    }
	}

	rest_length = req->param.par_read.length;
	for (i = 0; rest_length > 0;
	    rest_length -= rlength, i += rlength) {
	    /* MAX_BODY_SIZE 毎にバッファに読み込み */
	    len =
		rest_length >
		MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length;
	    errno =
		sfs_read_device(fp->f_inode->i_dev, buf,
				    fp->f_offset + i, len, &rlength);
	    if (errno)
		break;

	    /* 呼び出したプロセスのバッファへの書き込み */
	    errno = vput_reg(req->caller, req->param.par_read.buf + i,
			rlength, buf);
	    if (errno || (rlength < len)) {
		i += rlength;
		break;
	    }
	}
	if (errno) {
	    put_response(rdvno, req, errno, -1, 0, 0);
	    return (FAIL);
	}

	fp->f_offset += i;
	put_response(rdvno, req, EP_OK, i, 0, 0);
	return (SUCCESS);
    }
#ifdef DEBUG
    printk
	("posix:read: inode = 0x%x, offset = %d, buf = 0x%x, length = %d\n",
	 fp->f_inode, fp->f_offset, req->param.par_read.buf,
	 req->param.par_read.length);
#endif

    if (fp->f_offset >= fp->f_inode->i_size) {
	put_response(rdvno, req, EP_OK, 0, 0, 0);
	return (SUCCESS);
    }
    for (i = 0, rest_length = req->param.par_read.length;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	/* MAX_BODY_SIZE 毎にファイルに読み込み */
	len = rest_length > MAX_BODY_SIZE ? MAX_BODY_SIZE : rest_length;
	errno = fs_read_file(fp->f_inode,
			     fp->f_offset + i, buf, len, &rlength);
	if (errno) {
	    break;
	}

	/* 呼び出したプロセスのバッファへの書き込み */
	errno = vput_reg(req->caller, req->param.par_read.buf + i,
			 rlength, buf);
	if (errno || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }
    if (errno) {
	put_response(rdvno, req, errno, -1, 0, 0);
	return (FAIL);
    }
    fp->f_offset += i;
    put_response(rdvno, req, EP_OK, i, 0, 0);
    return (SUCCESS);
}
