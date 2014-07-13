/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/*
 * $Log: chdir.c,v $
 * Revision 1.3  2000/05/20 12:39:21  naniwa
 * to check as a directry
 *
 * Revision 1.2  2000/05/20 11:57:17  naniwa
 * minor fix
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include <fcntl.h>
#include <nerve/kcall.h>
#include "fs.h"

W psc_chdir_f(RDVNO rdvno, struct posix_request *req)
{
    struct inode *oldip;
    B path[MAX_NAMELEN];
    struct inode *startip;
    struct inode *ipp;
    struct access_info acc;
    W err;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    if (kcall->region_get(req->caller, req->param.par_chmod.path,
		 req->param.par_chmod.pathlen + 1, path)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    err = proc_get_cwd(req->procid, &oldip);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    if (*path == '/') {
	/* 絶対パスによる指定 */
	startip = rootfile;
    } else {
#ifdef notdef
	if (proc_get_cwd(req->procid, &startip)) {
	    put_response(rdvno, EINVAL, -1, 0);
	    return (FALSE);
	}
#else
	startip = oldip;
#endif
    }


    if (proc_get_uid(req->procid, &acc.uid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    if (proc_get_gid(req->procid, &acc.gid)) {
	put_response(rdvno, EINVAL, -1, 0);
	return (FALSE);
    }

    err = fs_lookup(startip, path, O_RDONLY, 0, &acc, &ipp);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    if ((ipp->i_mode & S_IFMT) != S_IFDIR) {
	/* ファイルは、ディレクトリではなかった。
	 * エラーとする
	 * 
	 */
	fs_close_file(ipp);
	put_response(rdvno, ENOTDIR, -1, 0);
	return (FALSE);
    }

    err = permit(ipp, &acc, X_OK);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    err = proc_set_cwd(req->procid, ipp);
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    dealloc_inode(oldip);
    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

/* psc_umask_f - umask の設定
 */
W
psc_umask_f (RDVNO rdvno, struct posix_request *req)
{
  W	error_no;
  W	old_umask;


  /* 古い umask の値を取り出す。
   * (システムコールの返り値として使用する)
   */
  error_no = proc_get_umask (req->procid, &old_umask);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }
  
  /* 新しい umask の値を設定する
   */
  error_no = proc_set_umask (req->procid, req->param.par_umask.umask);
  if (error_no)
    {
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  put_response (rdvno, EOK, old_umask, 0);
  return (TRUE);
}  
