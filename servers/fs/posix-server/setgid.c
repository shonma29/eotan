/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: setgid.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

/* psc_setgid_f - プロセスのグループ ID を設定する
 */
W
psc_setgid_f (RDVNO rdvno, struct posix_request *req)
{
  W	uid;
  W	errno;

  errno = proc_get_euid (req->procid, &uid);
  if (errno)
    {
      put_response (rdvno, req, errno, 0, -1, 0);
      return (FAIL);
    }

  if (uid != 0)
    {
      put_response (rdvno, req, EP_PERM, errno, -1, 0);
      return (FAIL);
    }

  errno = proc_set_egid (req->procid, req->param.par_setgid.gid);
  if (errno)
    {
      put_response (rdvno, req, errno, 0, -1, 0);
      return (FAIL);
    }

  put_response (rdvno, req, EP_OK, 0, 0, 0);
  return (SUCCESS);
}  
