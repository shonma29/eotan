/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: geteuid.c,v $
 * Revision 1.2  1999/03/21 14:49:34  monaka
 * Now works well...maybe. You know I'm honest when you got fork/exec system call.
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_geteuid_f (RDVNO rdvno, struct posix_request *req)
{
  W	err;
  W	euid;

  err = proc_get_euid (req->procid, &euid);
  if (err)
    {
      put_response (rdvno, err, -1, 0);
      return (FAIL);
    }
  
  put_response (rdvno, EP_OK, euid, 0);
  return (SUCCESS);
}  
