/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: getgid.c,v $
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_getgid_f (RDVNO rdvno, struct posix_request *req)
{
  W	err;
  W	gid;

  err = proc_get_gid (req->procid, &gid);
  if (err)
    {
      put_response (rdvno, err, -1, 0);
      return (FAIL);
    }
  
  put_response (rdvno, EP_OK, gid, 0);
  return (SUCCESS);
}  
