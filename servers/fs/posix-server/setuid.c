/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: setuid.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_setuid_f (RDVNO rdvno, struct posix_request *req)
{
  W	uid;
  W	errno;

  errno = proc_get_euid (req->procid, &uid);
  if (errno)
    {
      put_response (rdvno, errno, 0, -1);
      return (FALSE);
    }

  if (uid != 0)
    {
      put_response (rdvno, EPERM, errno, -1);
      return (FALSE);
    }

  errno = proc_set_euid (req->procid, req->param.par_setuid.uid);
  if (errno)
    {
      put_response (rdvno, errno, 0, -1);
      return (FALSE);
    }

  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}  
