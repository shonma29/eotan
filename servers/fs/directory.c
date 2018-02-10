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
#include <sys/syslimits.h>
#include "fs.h"
#include "api.h"

int if_link(fs_request *req)
{
    B src[NAME_MAX + 1];
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
    src[NAME_MAX] = '\0';
    error_no = kcall->region_copy(caller, (UB*)(req->packet.args.arg2),
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[NAME_MAX] = '\0';

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
  vnode_t	*startip;
  vnode_t	*newip;
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

  error_no = vfs_mkdir (startip, req->buf,
		       req->packet.args.arg2,
		       &acc,
		       &newip);
  if (error_no)
      /* ファイルがオープンできない */
      return error_no;
  
  vnodes_remove (newip);
  put_response (req->rdvno, EOK, 0, 0);
  return EOK;
}  

/* if_rmdir - ディレクトリを削除する
 */
int
if_rmdir (fs_request *req)
{
  W		error_no;
  vnode_t	*startip;
  struct permission	acc;

  error_no = session_get_path(&startip, req->packet.procid,
		    get_rdv_tid(req->rdvno), (UB*)(req->packet.args.arg1),
		    (UB*)(req->buf));
  if (error_no)
    return error_no;

  error_no = proc_get_permission (req->packet.procid, &acc);
  if (error_no)
      return error_no;

  error_no = vfs_rmdir (startip,
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
  vnode_t		*startip;
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

  error_no = vfs_remove (startip,
			  req->buf,
			  &acc);
  if (error_no)
      /* ファイルがオープンできない */
      return error_no;
  
  put_response (req->rdvno, EOK, 0, 0);
  return EOK;
} 
