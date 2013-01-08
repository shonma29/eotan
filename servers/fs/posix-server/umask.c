/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: umask.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

/* psc_umask_f - umask の設定
 */
W
psc_umask_f (RDVNO rdvno, struct posix_request *req)
{
  W	errno;
  W	old_umask;


  /* 古い umask の値を取り出す。
   * (システムコールの返り値として使用する)
   */
  errno = proc_get_umask (req->procid, &old_umask);
  if (errno)
    {
      put_response (rdvno, req, errno, -1, 0, 0);
      return (FAIL);
    }
  
  /* 新しい umask の値を設定する
   */
  errno = proc_set_umask (req->procid, req->param.par_umask.umask);
  if (errno)
    {
      put_response (rdvno, req, errno, -1, 0, 0);
      return (FAIL);
    }

  put_response (rdvno, req, EP_OK, old_umask, 0, 0);
  return (SUCCESS);
}  
