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
#include <sys/errno.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "api.h"
#include "vfs.h"
#include "procfs/process.h"

int if_close(fs_request *req)
{
    struct file *fp;
    W err;

    err = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (err)
	return err;

    err = vnodes_remove(fp->f_inode);
    if (err)
	return err;

    fp->f_inode = NULL;
    reply2(req->rdvno, 0, 0, 0);
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
  error_no = session_get_opened_file (req->packet.process_id, req->packet.arg1, &fp);
  if (error_no)
      return error_no;

  error_no = proc_get_file (req->packet.process_id, req->packet.arg2, &fp2);
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
  error_no = proc_set_file(req->packet.process_id, req->packet.arg2,
			fp->f_omode, fp->f_inode);
  if (error_no)
      return error_no;

  reply2(req->rdvno, 0, req->packet.arg2, 0);
  return EOK;
}  

int if_lseek(fs_request *req)
{
    struct file *fp;
    W error_no;
    off_t *offp = (off_t*)&(req->packet.arg2);

    error_no = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (error_no)
	return error_no;

    switch (req->packet.arg4) {
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

    reply64(req->rdvno, EOK, fp->f_offset);
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

    error_no = proc_alloc_fileid(req->packet.process_id, &fileid);
    if (error_no)
	/* メモリ取得エラー */
	return ENOMEM;

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = session_get_path(&startip, req->packet.process_id,
		    get_rdv_tid(req->rdvno), (UB*)(req->packet.arg1),
		    (UB*)(req->buf));
    if (error_no)
	return error_no;

    error_no = proc_get_permission(req->packet.process_id, &acc);
    if (error_no)
	return error_no;

    error_no = vfs_open(startip, req->buf,
			 req->packet.arg2,
			 req->packet.arg3,
			 &acc, &newip);
    if (error_no)
	/* ファイルがオープンできない */
	return error_no;

    if ((newip->mode & S_IFMT) == S_IFDIR) {
	/* ファイルは、ディレクトリだった
	 * エラーとする
	 */
	if (req->packet.arg2 != O_RDONLY) {
	    vnodes_remove(newip);
	    return EISDIR;
	}
    }

    if (proc_set_file(req->packet.process_id, fileid,
		      req->packet.arg2, newip)) {
	vnodes_remove(newip);
	return EINVAL;
    }

    reply2(req->rdvno, 0, fileid, 0);
    return EOK;
}

/* if_read - ファイルからのデータの読み込み
 */
int if_read(fs_request *req)
{
    W error_no;
    struct file *fp;
    size_t rlength;
    W rest_length;
    W i, len;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
    if (error_no)
	return error_no;

    if (fp->f_omode == O_WRONLY)
	return EBADF;

    int offset = fp->f_offset;
    for (i = 0, rest_length = req->packet.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	/* MAX_BODY_SIZE 毎にファイルに読み込み */
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	int delta = fs_read(fp->f_inode, req->buf, offset, len, &rlength);
	if (delta < 0) {
	    return (-delta);
	} else if (!rlength)
	    break;

	/* 呼び出したプロセスのバッファへの書き込み */
	error_no = kcall->region_put(caller, (UB*)(req->packet.arg2) + i,
			 rlength, req->buf);
	if (error_no)
	    return EFAULT;

	offset += delta;
    }

    fp->f_offset = offset;
    reply2(req->rdvno, 0, i, 0);
    return EOK;
}

int if_write(fs_request *req)
{
    W error_no;
    struct file *fp;
    size_t rlength;
    W i, len;
    W rest_length;

    error_no = session_get_opened_file(req->packet.process_id, req->packet.arg1, &fp);
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
	error_no = vfs_write(fp->f_inode, req->buf,
			      fp->f_inode->size, len, &rlength);
	if (error_no || (rlength < len)) {
	  break;
	}
      }
    }
    if (error_no)
	return error_no;

    for (i = 0, rest_length = req->packet.arg3;
	 rest_length > 0; rest_length -= rlength, i += rlength) {
	len = rest_length > sizeof(req->buf) ? sizeof(req->buf) : rest_length;
	error_no =
	    kcall->region_get(get_rdv_tid(req->rdvno), (UB*)(req->packet.arg2) + i, len, req->buf);
	if (error_no)
	    break;

	error_no = vfs_write(fp->f_inode, req->buf,
			      fp->f_offset + i, len, &rlength);
	if (error_no || (rlength < len)) {
	    i += rlength;
	    break;
	}
    }

    if (error_no)
	return error_no;

    fp->f_offset += i;
    reply2(req->rdvno, 0, i, 0);
    return EOK;
}
