/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: mkdir.c,v $
 * Revision 1.2  2000/05/06 03:54:26  naniwa
 * implement mkdir/rmdir, etc.
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_mkdir_f (RDVNO rdvno, struct posix_request *req)
{
  B		pathname[MAX_NAMELEN];
  W		fileid;
  W		errno;
  struct inode	*startip;
  struct inode	*newip;
  struct access_info	acc;
  W		umask;

  errno = proc_alloc_fileid (req->procid, &fileid);
  if (errno)
    {
      /* メモリ取得エラー */
      put_response (rdvno, ENOMEM, -1, 0);
      return (FALSE);
    }

  errno = vget_reg (req->caller, req->param.par_mkdir.path,
		    req->param.par_mkdir.pathlen + 1, pathname);
  if (errno)
    {
      /* パス名のコピーエラー */
      if (errno == E_PAR)
	put_response (rdvno, EINVAL, -1, 0);
      else
	put_response (rdvno, EFAULT, -1, 0);
	
      return (FALSE);
    }

  if (*pathname != '/')
    {
      errno = proc_get_cwd (req->procid, &startip);
      if (errno)
	{
	  put_response (rdvno, errno, -1, 0);
	  return (FALSE);
	}
    }
  else
    {
      startip = rootfile;
    }
  errno = proc_get_uid (req->procid, &(acc.uid));
  if (errno)
    {
      put_response (rdvno, errno, -1, 0);
      return (FALSE);
    }

  errno = proc_get_gid (req->procid, &(acc.gid));
  if (errno)
    {
      put_response (rdvno, errno, -1, 0);
      return (FALSE);
    }

  errno = proc_get_umask (req->procid, &umask);
  if (errno)
    {
      put_response (rdvno, errno, -1, 0);
      return (FALSE);
    }

  errno = fs_make_dir (startip, pathname,
		       (req->param.par_mkdir.mode & (~umask)),
		       &acc,
		       &newip);
  if (errno)
    {
      /* ファイルがオープンできない */
      put_response (rdvno, errno, -1, 0);
      return (FALSE);
    }
  
  fs_close_file (newip);
  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}  
