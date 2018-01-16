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

int if_close(fs_request *req)
{
    struct file *fp;
    W err;

    err = session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (err)
	return err;

    err = vnodes_remove(fp->f_inode);
    if (err)
	return err;

    fp->f_inode = NULL;
    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}

/* if_dup - ファイル記述子の複製
 */
int
if_dup (fs_request *req)
{
  W		error_no;
  struct file	*fp;
  W		newfileid;


  /* プロセスからファイル構造体へのポインタを取り出す
   */
  error_no = session_get_opened_file (req->packet.procid, req->packet.args.arg1, &fp);
  if (error_no)
      return error_no;

  error_no = proc_alloc_fileid (req->packet.procid, &newfileid);
  if (error_no)
      return error_no;

  fp->f_inode->refer_count++;
  error_no = proc_set_file (req->packet.procid, newfileid, fp->f_omode, fp->f_inode);
  if (error_no)
      return error_no;

  put_response (req->rdvno, EOK, newfileid, 0);
  return EOK;
}  

/* if_dup2 -ファイル記述子の複製
 */
int
if_dup2 (fs_request *req)
{
  W		error_no;
  struct file	*fp, *fp2;


  /* プロセスからファイル構造体へのポインタを取り出す
   */
  error_no = session_get_opened_file (req->packet.procid, req->packet.args.arg1, &fp);
  if (error_no)
      return error_no;

  error_no = proc_get_file (req->packet.procid, req->packet.args.arg2, &fp2);
  if (error_no)
    return error_no;

  if (fp2->f_inode != NULL) {
    /* 既に open されている file id だった */
    error_no = vnodes_remove (fp2->f_inode);
    if (error_no)
      return error_no;

    fp2->f_inode = NULL;
  }
  fp->f_inode->refer_count++;
  error_no = proc_set_file(req->packet.procid, req->packet.args.arg2,
			fp->f_omode, fp->f_inode);
  if (error_no)
      return error_no;

  put_response (req->rdvno, EOK, req->packet.args.arg2, 0);
  return EOK;
}  

int if_lseek(fs_request *req)
{
    struct file *fp;
    W error_no;
    off_t *offp = (off_t*)&(req->packet.args.arg2);

    error_no = session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

    switch (req->packet.args.arg4) {
    case SEEK_SET:
	fp->f_offset = *offp;
	break;

    case SEEK_CUR:
	fp->f_offset += *offp;
	break;

    case SEEK_END:
	fp->f_offset = fp->f_inode->size + *offp;
	break;

    default:
	return EINVAL;
    }

    if (fp->f_offset < 0) {
	fp->f_offset = 0;
    }
    else if (fp->f_inode->mode & S_IFCHR) {
      if (fp->f_offset > fp->f_inode->size) {
	/* ブロックデバイスなど，サイズの制限のあるデバイスの場合 */
	fp->f_offset = fp->f_inode->size;
      }
    }

    put_response_long(req->rdvno, EOK, fp->f_offset);
    return EOK;
}

/* if_open - ファイルのオープン
 */
int if_open(fs_request *req)
{
    W fileid;
    W error_no;
    vnode_t *startip;
    vnode_t *newip;
    struct permission acc;

    error_no = proc_alloc_fileid(req->packet.procid, &fileid);
    if (error_no)
	/* メモリ取得エラー */
	return ENOMEM;

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = session_get_path(&startip, req->packet.procid,
		    get_rdv_tid(req->rdvno), (UB*)(req->packet.param.par_open.path),
		    (UB*)(req->buf));
    if (error_no)
	return error_no;

    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    error_no = fs_open_file(req->buf,
			 req->packet.param.par_open.oflag,
			 req->packet.param.par_open.mode,
			 &acc, startip, &newip);
    if (error_no)
	/* ファイルがオープンできない */
	return error_no;

    if ((newip->mode & S_IFMT) == S_IFDIR) {
	/* ファイルは、ディレクトリだった
	 * エラーとする
	 */

	/* root ユーザの場合には、
	 * 成功でもよい
	 */
	if (acc.uid != ROOT_UID) {
	    vnodes_remove(newip);
	    return EACCES;
	}

	if (req->packet.param.par_open.oflag != O_RDONLY) {
	    vnodes_remove(newip);
	    return EISDIR;
	}
    }

    if (proc_set_file(req->packet.procid, fileid,
		      req->packet.param.par_open.oflag, newip)) {
	vnodes_remove(newip);
	return EINVAL;
    }

    put_response(req->rdvno, EOK, fileid, 0);
    return EOK;
}

/* if_read - ファイルからのデータの読み込み
 */
int if_read(fs_request *req)
{
    W error_no;
    struct file *fp;
    W rlength;
    W rest_length;
    W i, len;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

    if (fp->f_omode == O_WRONLY)
	return EBADF;

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
    if (error_no)
	return error_no;

    fp->f_offset += i;
    put_response(req->rdvno, EOK, i, 0);
    return EOK;
}

int if_write(fs_request *req)
{
    W error_no;
    struct file *fp;
    W rlength;
    W i, len;
    W rest_length;

    error_no = session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

    if (fp->f_omode == O_RDONLY)
	return EBADF;

    if ((! (fp->f_inode->mode & S_IFCHR)) &&
	(fp->f_offset > fp->f_inode->size)) {
      /* 通常ファイルで，書き込む場所がファイルの内容が存在しない場所 */
      /* そこまでを 0 で埋める */
      memset(req->buf, 0, sizeof(req->buf));
      for (rest_length = fp->f_offset - fp->f_inode->size;
	   rest_length > 0; rest_length -= rlength) {
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no = fs_write_file(fp->f_inode,
			      fp->f_inode->size, req->buf, len, &rlength);
	if (error_no || (rlength < len)) {
	  break;
	}
      }
    }
    if (error_no)
	return error_no;

    for (i = 0, rest_length = req->packet.args.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no =
	    kcall->region_get(get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg2) + i, len, req->buf);
	if (error_no)
	    break;

	error_no = fs_write_file(fp->f_inode,
			      fp->f_offset + i, req->buf, len, &rlength);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }

    if (error_no)
	return error_no;

    fp->f_offset += i;
    put_response(req->rdvno, EOK, i, 0);
    return EOK;
}
