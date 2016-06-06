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

#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/stat.h>
#include "fs.h"
#include "api.h"

int if_getdents(fs_request *req)
{
    W error_no;
    struct file *fp;
    W len, flen;

    error_no =
	session_get_opened_file(req->packet.procid, req->packet.args.arg1, &fp);
    if (error_no)
	return error_no;

    /* 対象ファイルが
     * ディレクトリ以外の場合には、エラーにする
     */
    if ((fp->f_inode->i_mode & S_IFMT) != S_IFDIR)
	return EINVAL;

    error_no = fs_getdents(fp->f_inode, get_rdv_tid(req->rdvno), fp->f_offset,
			(UB*)(req->packet.args.arg2),
			req->packet.args.arg3, &len, &flen);

    if (error_no)
	return error_no;

    fp->f_offset += flen;
    put_response(req->rdvno, EOK, len, 0);
    return EOK;
}

int if_link(fs_request *req)
{
    B src[MAX_NAMELEN + 1];
    struct permission acc;
    W error_no;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = kcall->region_copy(caller, (UB*)(req->packet.args.arg1),
		     sizeof(src) - 1, src);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    src[MAX_NAMELEN] = '\0';
    error_no = kcall->region_copy(caller, (UB*)(req->packet.args.arg2),
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[MAX_NAMELEN] = '\0';

    /* プロセスのユーザ ID とグループ ID の
     * 取り出し。
     * この情報に基づいて、ファイルを削除できるかどうかを
     * 決定する。
     */
    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    error_no = fs_link_file(req->packet.procid, src, req->buf, &acc);
    if (error_no)
	return error_no;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}

int
if_mkdir (fs_request *req)
{
  W		fileid;
  W		error_no;
  struct inode	*startip;
  struct inode	*newip;
  struct permission	acc;

  error_no = proc_alloc_fileid (req->packet.procid, &fileid);
  if (error_no)
      /* メモリ取得エラー */
      return ENOMEM;

  error_no = session_get_path(&startip, req->packet.procid,
		    get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		    (UB*)(req->buf));
  if (error_no)
    return error_no;

  error_no = proc_get_permission (req->packet.procid, &acc);
  if (error_no)
      return error_no;

  error_no = fs_create_dir (startip, req->buf,
		       req->packet.args.arg2,
		       &acc,
		       &newip);
  if (error_no)
      /* ファイルがオープンできない */
      return error_no;
  
  fs_close_file (newip);
  put_response (req->rdvno, EOK, 0, 0);
  return EOK;
}  

/* if_rmdir - ディレクトリを削除する
 */
int
if_rmdir (fs_request *req)
{
  W		error_no;
  struct inode	*startip;
  struct permission	acc;

  error_no = session_get_path(&startip, req->packet.procid,
		    get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		    (UB*)(req->buf));
  if (error_no)
    return error_no;

  error_no = proc_get_permission (req->packet.procid, &acc);
  if (error_no)
      return error_no;

  error_no = fs_remove_dir (startip,
			 req->buf,
			 &acc);
  if (error_no)
      /* ファイルがオープンできない */
      return error_no;
  
  put_response (req->rdvno, EOK, 0, 0);
  return EOK;
}  

/* if_unlink - ファイルを削除する
 *
 * 引数:
 * 	req->param.par_unlink.path	削除するファイルのパス名
 *	req->param.par_unlink.pathlen	パス名の長さ
 *
 */
int
if_unlink (fs_request *req)
{
  W			error_no;
  struct inode		*startip;
  struct permission	acc;

  /* パス名を呼び出し元のプロセス(タスク)から
   * 取り出す。
   * 呼び出し元のタスク ID は、メッセージパラメータの
   * 中に入っている。
   */
  error_no = session_get_path(&startip, req->packet.procid,
		    get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		    (UB*)(req->buf));
  if (error_no)
    return error_no;

  /* プロセスのユーザ ID とグループ ID の
   * 取り出し。
   * この情報に基づいて、ファイルを削除できるかどうかを
   * 決定する。
   */
  error_no = proc_get_permission (req->packet.procid, &acc);
  if (error_no)
      return error_no;

  error_no = fs_remove_file (startip,
			  req->buf,
			  &acc);
  if (error_no)
      /* ファイルがオープンできない */
      return error_no;
  
  put_response (req->rdvno, EOK, 0, 0);
  return EOK;
} 
