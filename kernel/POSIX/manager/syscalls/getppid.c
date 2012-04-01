/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: getppid.c,v $
 * Revision 1.2  1999/03/21 14:49:38  monaka
 * Now works well...maybe. You know I'm honest when you got fork/exec system call.
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W
psc_getppid_f (struct posix_request *req)
{
  W	err;
  W	ppid;

  err = proc_get_ppid (req->procid, &ppid);
  if (err)
    {
      put_response (req, err, -1, 0, 0);
      return (FAIL);
    }
  
  put_response (req, EP_OK, ppid, 0, 0);
  return (SUCCESS);
}  

