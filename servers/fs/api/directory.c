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

#include <nerve/kcall.h>
#include "fs.h"

W psc_getdents_f(RDVNO rdvno, struct posix_request *req)
{
    W error_no;
    struct file *fp;
    W len, flen;

    error_no =
	proc_get_file(req->procid, req->param.par_getdents.fileid, &fp);
    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
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

    error_no = fs_getdents(fp->f_inode, req->caller, fp->f_offset,
			req->param.par_getdents.buf,
			req->param.par_getdents.length, &len, &flen);

    if (error_no) {
	put_response(rdvno, error_no, -1, 0);
    }

    fp->f_offset += flen;
    put_response(rdvno, EOK, len, 0);
    return (TRUE);
}

W psc_link_f(RDVNO rdvno, struct posix_request *req)
{
    B src[MAX_NAMELEN], dst[MAX_NAMELEN];
    struct access_info acc;
    W error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    error_no = kcall->region_get(req->caller, req->param.par_link.src,
		     req->param.par_link.srclen + 1, src);
    if (error_no) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }
    error_no = kcall->region_get(req->caller, req->param.par_link.dst,
		     req->param.par_link.dstlen + 1, dst);
    if (error_no) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }

    /* プロセスのユーザ ID とグループ ID の
     * 取り出し。
     * この情報に基づいて、ファイルを削除できるかどうかを
     * 決定する。
     */
    error_no = proc_get_uid(req->procid, &(acc.uid));
    if (error_no) {
	put_response(rdvno, error_no, 0, 0);
	return (FALSE);
    }
    error_no = proc_get_gid(req->procid, &(acc.gid));
    if (error_no) {
	put_response(rdvno, error_no, 0, 0);
	return (FALSE);
    }

    error_no = fs_link_file(req->procid, src, req->param.par_link.srclen,
			 dst, req->param.par_link.dstlen, &acc);
    if (error_no) {
	put_response(rdvno, error_no, 0, 0);
	return (FALSE);
    }
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

W
psc_mkdir_f (RDVNO rdvno, struct posix_request *req)
{
  B		pathname[MAX_NAMELEN];
  W		fileid;
  W		error_no;
  struct inode	*startip;
  struct inode	*newip;
  struct access_info	acc;
  W		umask;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  error_no = proc_alloc_fileid (req->procid, &fileid);
  if (error_no)
    {
      /* メモリ取得エラー */
      put_response (rdvno, ENOMEM, -1, 0);
      return (FALSE);
    }

  error_no = kcall->region_get(req->caller, req->param.par_mkdir.path,
		    req->param.par_mkdir.pathlen + 1, pathname);
  if (error_no)
    {
      /* パス名のコピーエラー */
      if (error_no == E_PAR)
	put_response (rdvno, EINVAL, -1, 0);
      else
	put_response (rdvno, EFAULT, -1, 0);
	
      return (FALSE);
    }

  if (*pathname != '/')
    {
      error_no = proc_get_cwd (req->procid, &startip);
      if (error_no)
	{
	  put_response (rdvno, error_no, -1, 0);
	  return (FALSE);
	}
    }
  else
    {
      startip = rootfile;
    }
  error_no = proc_get_uid (req->procid, &(acc.uid));
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  error_no = proc_get_gid (req->procid, &(acc.gid));
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  error_no = proc_get_umask (req->procid, &umask);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  error_no = fs_make_dir (startip, pathname,
		       (req->param.par_mkdir.mode & (~umask)),
		       &acc,
		       &newip);
  if (error_no)
    {
      /* ファイルがオープンできない */
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }
  
  fs_close_file (newip);
  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}  

/* psc_rmdir_f - ディレクトリを削除する
 */
W
psc_rmdir_f (RDVNO rdvno, struct posix_request *req)
{
  B		pathname[MAX_NAMELEN];
  W		error_no;
  struct inode	*startip;
  struct access_info	acc;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  error_no = kcall->region_get(req->caller, req->param.par_rmdir.path,
		    req->param.par_rmdir.pathlen + 1, pathname);
  if (error_no)
    {
      /* パス名のコピーエラー */
      if (error_no == E_PAR)
	put_response (rdvno, EINVAL, -1, 0);
      else
	put_response (rdvno, EFAULT, -1, 0);
	
      return (FALSE);
    }


  if (*pathname != '/')
    {
      error_no = proc_get_cwd (req->procid, &startip);
      if (error_no)
	{
	  put_response (rdvno, error_no, -1, 0);
	  return (FALSE);
	}
    }
  else
    {
      startip = rootfile;
    }
  error_no = proc_get_uid (req->procid, &(acc.uid));
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  error_no = proc_get_gid (req->procid, &(acc.gid));
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  error_no = fs_remove_dir (startip,
			 pathname,
			 &acc);
  if (error_no)
    {
      /* ファイルがオープンできない */
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }
  
  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}  

/* psc_unlink_f - ファイルを削除する
 *
 * 引数:
 * 	req->param.par_unlink.path	削除するファイルのパス名
 *	req->param.par_unlink.pathlen	パス名の長さ
 *
 */
W
psc_unlink_f (RDVNO rdvno, struct posix_request *req)
{
  B			pathname[MAX_NAMELEN];
  W			error_no;
  struct inode		*startip;
  struct access_info	acc;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  /* パス名を呼び出し元のプロセス(タスク)から
   * 取り出す。
   * 呼び出し元のタスク ID は、メッセージパラメータの
   * 中に入っている。
   */
  error_no = kcall->region_get(req->caller,
		    req->param.par_unlink.path,
		    req->param.par_unlink.pathlen + 1,
		    pathname);
  if (error_no)
    {
      /* パス名のコピーエラー */
      if (error_no == E_PAR)
	put_response (rdvno, EINVAL, 0, 0);
      else
	put_response (rdvno, EFAULT, 0, 0);
	
      return (FALSE);
    }


  /* ファイルが相対パスで指定されているかどうかを
   * チェックする。
   * (パス名が '/' ではじまっていない場合には、
   * 相対パスと見なす)
   * 相対パスではじまっている場合には、
   * プロセスのカレントディレクトリを取り出して、
   * そこをファイルのパスの最初のディレクトリと
   * する。
   */
  if (*pathname != '/')
    {
      error_no = proc_get_cwd (req->procid, &startip);
      if (error_no)
	{
	  put_response (rdvno, error_no, 0, 0);
	  return (FALSE);
	}
    }
  else
    {
      startip = rootfile;
    }


  /* プロセスのユーザ ID とグループ ID の
   * 取り出し。
   * この情報に基づいて、ファイルを削除できるかどうかを
   * 決定する。
   */
  error_no = proc_get_uid (req->procid, &(acc.uid));
  if (error_no)
    {
      put_response (rdvno, error_no, 0, 0);
      return (FALSE);
    }
  error_no = proc_get_gid (req->procid, &(acc.gid));
  if (error_no)
    {
      put_response (rdvno, error_no, 0, 0);
      return (FALSE);
    }

  error_no = fs_remove_file (startip,
			  pathname,
			  &acc);
  if (error_no)
    {
      /* ファイルがオープンできない */
      put_response (rdvno, error_no, 0, 0);
      return (FALSE);
    }
  
  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
} 
