/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: getpid.c,v $
 * Revision 1.2  1999/03/21 14:49:37  monaka
 * Now works well...maybe. You know I'm honest when you got fork/exec system call.
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_getpid_f (RDVNO rdvno, struct posix_request *req)
{
  W	err;
  W	pid;

  err = proc_get_pid (req->procid, &pid);
  if (err)
    {
      put_response (rdvno, err, -1, 0);
      return (FALSE);
    }
  
  put_response (rdvno, EOK, pid, 0);
  return (TRUE);
}  
