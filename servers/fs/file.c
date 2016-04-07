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

#include <device.h>
#include <fcntl.h>
#include <string.h>
#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/stat.h>
#include "fs.h"
#include "api.h"

void if_close(fs_request *req)
{
    struct file *fp;
    W err;

    err = proc_get_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (err) {
	put_response(req->rdvno, err, -1, 0);
	return;
    }

    if (fp->f_inode == NULL) {
	put_response(req->rdvno, EBADF, -1, 0);
	return;
    }

    err = fs_close_file(fp->f_inode);
    if (err) {
	put_response(req->rdvno, err, -1, 0);
	return;
    }

    fp->f_inode = NULL;
    put_response(req->rdvno, EOK, 0, 0);
}

/* if_dup - ファイル記述子の複製
 */
void
if_dup (fs_request *req)
{
  W		error_no;
  struct file	*fp;
  W		newfileid;


  /* プロセスからファイル構造体へのポインタを取り出す
   */
  error_no = proc_get_file (req->packet.procid, req->packet.args.arg1, &fp);
  if (error_no)
    {
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }


  if (fp->f_inode == NULL)
    {
      /* 複製するファイル記述子の番号がおかしかった
       */
      put_response (req->rdvno, EBADF, -1, 0);
      return;
    }

  error_no = proc_alloc_fileid (req->packet.procid, &newfileid);
  if (error_no)
    {
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

  fp->f_inode->i_refcount++;
  error_no = proc_set_file (req->packet.procid, newfileid, fp->f_omode, fp->f_inode);
  if (error_no)
    {
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

  put_response (req->rdvno, EOK, newfileid, 0);
}  

/* if_dup2 -ファイル記述子の複製
 */
void
if_dup2 (fs_request *req)
{
  W		error_no;
  struct file	*fp, *fp2;


  /* プロセスからファイル構造体へのポインタを取り出す
   */
  error_no = proc_get_file (req->packet.procid, req->packet.args.arg1, &fp);
  if (error_no)
    {
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }


  if (fp->f_inode == NULL)
    {
      /* 複製するファイル記述子の番号がおかしい。
       * (ファイルをオープンしていない)
       */
      put_response (req->rdvno, EBADF, -1, 0);
      return;
    }

  error_no = proc_get_file (req->packet.procid, req->packet.args.arg2, &fp2);
  if (error_no) {
    put_response (req->rdvno, error_no, -1, 0);
    return;
  }
  if (fp2->f_inode != NULL) {
    /* 既に open されている file id だった */
    error_no = fs_close_file (fp2->f_inode);
    if (error_no) {
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }
    fp2->f_inode = NULL;
  }
  fp->f_inode->i_refcount++;
  error_no = proc_set_file(req->packet.procid, req->packet.args.arg2,
			fp->f_omode, fp->f_inode);
  if (error_no)
    {
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

  put_response (req->rdvno, EOK, req->packet.args.arg2, 0);
}  

/* if_fcntl - ファイルに対して特殊な操作を行う。
 */
void if_fcntl(fs_request * req)
{
    W error_no;
    struct file *fp;

    error_no = proc_get_file(req->packet.procid, req->packet.param.par_fcntl.fileid, &fp);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    } else if (fp == 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    } else if (fp->f_inode == 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    /* とりあえず、サポートしていないというエラーで返す
     */
    put_response(req->rdvno, ENOTSUP, 0, 0);
}

void if_lseek(fs_request *req)
{
    struct file *fp;
    W error_no;
    off_t *offp = (off_t*)&(req->packet.args.arg2);

    error_no = proc_get_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, error_no);
	return;
    }

    switch (req->packet.args.arg4) {
    case SEEK_SET:
	fp->f_offset = *offp;
	break;

    case SEEK_CUR:
	fp->f_offset += *offp;
	break;

    case SEEK_END:
	fp->f_offset = fp->f_inode->i_size + *offp;
	break;

    default:
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    if (fp->f_offset < 0) {
	fp->f_offset = 0;
    }
    else if (fp->f_inode->i_mode & S_IFCHR) {
      if (fp->f_offset > fp->f_inode->i_size) {
	/* ブロックデバイスなど，サイズの制限のあるデバイスの場合 */
	fp->f_offset = fp->f_inode->i_size;
      }
    }

    put_response_long(req->rdvno, EOK, fp->f_offset);
}

/* if_open - ファイルのオープン
 */
void if_open(fs_request *req)
{
    W fileid;
    W error_no;
    struct inode *startip;
    struct inode *newip;
    struct permission acc;

    error_no = proc_alloc_fileid(req->packet.procid, &fileid);
    if (error_no) {
	/* メモリ取得エラー */
	put_response(req->rdvno, ENOMEM, -1, 0);
	return;
    }

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = kcall->region_copy(get_rdv_tid(req->rdvno), req->packet.param.par_open.path,
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(req->rdvno, EINVAL, -1, 0);
	else
	    put_response(req->rdvno, EFAULT, -1, 0);

	return;
    }
    req->buf[MAX_NAMELEN] = '\0';
    if (req->buf[0] != '/') {
	error_no = proc_get_cwd(req->packet.procid, &startip);
	if (error_no) {
	    put_response(req->rdvno, error_no, -1, 0);
	    return;
	}
    } else {
	startip = rootfile;
    }
    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }

    error_no = fs_open_file(req->buf,
			 req->packet.param.par_open.oflag,
			 req->packet.param.par_open.mode,
			 &acc, startip, &newip);
    if (error_no) {
	/* ファイルがオープンできない */
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }

    if ((newip->i_mode & S_IFMT) == S_IFDIR) {
	/* ファイルは、ディレクトリだった
	 * エラーとする
	 */

	/* root ユーザの場合には、
	 * 成功でもよい
	 */
	if (acc.uid != SU_UID) {
	    fs_close_file(newip);
	    put_response(req->rdvno, EACCES, -1, 0);
	    return;
	}

	if (req->packet.param.par_open.oflag != O_RDONLY) {
	    fs_close_file(newip);
	    put_response(req->rdvno, EISDIR, -1, 0);
	    return;
	}
    }

    if (proc_set_file(req->packet.procid, fileid,
		      req->packet.param.par_open.oflag, newip)) {
	fs_close_file(newip);
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    put_response(req->rdvno, EOK, fileid, 0);
}

/* if_read - ファイルからのデータの読み込み
 */
void if_read(fs_request *req)
{
    W error_no;
    struct file *fp;
    W rlength;
    W rest_length;
    W i, len;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = proc_get_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    } else if (fp == 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    } else if (fp->f_inode == 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    if (fp->f_omode == O_WRONLY) {
	put_response(req->rdvno, EBADF, -1, 0);
	return;
    }

    if (fp->f_flag & F_PIPE) {
	/* パイプの読み書き */
	put_response(req->rdvno, ENOTSUP, -1, 0);
	return;
    }

    if (fp->f_inode->i_mode & S_IFCHR) {
	if ((fp->f_inode->i_dev & BLOCK_DEVICE_MASK)) {
	    /* ブロックデバイスだった */
	    if (fp->f_offset >= fp->f_inode->i_size) {
		error_no = EOK;
		req->packet.args.arg3 = 0;
	    }
	}

	rest_length = req->packet.args.arg3;
	for (i = 0; rest_length > 0;
	    rest_length -= rlength, i += rlength) {
	    /* MAX_BODY_SIZE 毎にバッファに読み込み */
	    len =
		rest_length >
		sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	    error_no =
		read_device(fp->f_inode->i_dev, req->buf,
				    fp->f_offset + i, len, &rlength);
	    if (error_no)
		break;

	    /* 呼び出したプロセスのバッファへの書き込み */
	    error_no = kcall->region_put(caller, (UB*)(req->packet.args.arg2) + i,
			rlength, req->buf);
	    if (error_no || (rlength < len)) {
		i += rlength;
		break;
	    }
	}
	if (error_no) {
	    put_response(req->rdvno, error_no, -1, 0);
	    return;
	}

	fp->f_offset += i;
	put_response(req->rdvno, EOK, i, 0);
	return;
    }
#ifdef DEBUG
    dbg_printf
	("fs: read: inode = 0x%x, offset = %d, buf = 0x%x, length = %d\n",
	 fp->f_inode, fp->f_offset, req->packet.args.arg2,
	 req->packet.args.arg3);
#endif

    if (fp->f_offset >= fp->f_inode->i_size) {
	put_response(req->rdvno, EOK, 0, 0);
	return;
    }
    for (i = 0, rest_length = req->packet.args.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	/* MAX_BODY_SIZE 毎にファイルに読み込み */
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no = fs_read_file(fp->f_inode,
			     fp->f_offset + i, req->buf, len, &rlength);
	if (error_no) {
	    break;
	}

	/* 呼び出したプロセスのバッファへの書き込み */
	error_no = kcall->region_put(caller, (UB*)(req->packet.args.arg2) + i,
			 rlength, req->buf);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }
    fp->f_offset += i;
    put_response(req->rdvno, EOK, i, 0);
}

void if_write(fs_request *req)
{
    W error_no;
    struct file *fp;
    W rlength;
    W i, len;
    W rest_length;
#ifdef DEBUG
    W j;
#endif

    error_no = proc_get_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    } else if (fp == 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    } else if (fp->f_inode == 0) {
	put_response(req->rdvno, EINVAL, -1, 0);
	return;
    }

    if (fp->f_omode == O_RDONLY) {
	put_response(req->rdvno, EBADF, -1, 0);
	return;
    }

    if (fp->f_flag & F_PIPE) {
	/* パイプの読み書き */
	put_response(req->rdvno, ENOTSUP, -1, 0);
	return;
    }
#ifdef debug
    dbg_printf
	("fs: write: inode = 0x%x, offset = %d, buf = 0x%x, length = %d\n",
	 fp->f_inode, fp->f_offset, req->packet.args.arg2,
	 req->packet.args.arg3);
#endif
    if ((! (fp->f_inode->i_mode & S_IFCHR)) &&
	(fp->f_offset > fp->f_inode->i_size)) {
      /* 通常ファイルで，書き込む場所がファイルの内容が存在しない場所 */
      /* そこまでを 0 で埋める */
      memset(req->buf, 0, sizeof(req->buf));
      for (rest_length = fp->f_offset - fp->f_inode->i_size;
	   rest_length > 0; rest_length -= rlength) {
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no = fs_write_file(fp->f_inode,
			      fp->f_inode->i_size, req->buf, len, &rlength);
	if (error_no || (rlength < len)) {
	  break;
	}
      }
    }
    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }

    for (i = 0, rest_length = req->packet.args.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
#ifdef DEBUG
	dbg_printf
	    ("fs: vget_reg (caller = %d, src addr = 0x%x, size = %d, dst = 0x%x\n",
	     req->caller, req->packet.args.arg2 + i,
	     rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length,
	     req->buf);
#endif
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no =
	    kcall->region_get(get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg2) + i, len, req->buf);
	if (error_no)
	    break;
#ifdef DEBUG
	dbg_printf("fs: writedata length = %d\n", len);
	for (j = 0;
	     j <
	     ((rest_length > sizeof(req->buf)) ? sizeof(req->buf) : rest_length);
	     j++) {
	    char print_buffer[2];
	    dbg_printf("fs: [%x]", req->buf[j]);
	}
	dbg_printf("\n");
#endif

	error_no = fs_write_file(fp->f_inode,
			      fp->f_offset + i, req->buf, len, &rlength);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }

    if (error_no) {
	put_response(req->rdvno, error_no, -1, 0);
	return;
    }

    fp->f_offset += i;
    put_response(req->rdvno, EOK, i, 0);
}