/*

B-Free Project $B$N@8@.J*$O(B GNU Generic PUBLIC LICENSE $B$K=>$$$^$9!#(B

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

#include "posix.h"

W
psc_getgid_f (struct posix_request *req)
{
  W	err;
  W	gid;

  err = proc_get_gid (req->procid, &gid);
  if (err)
    {
      put_response (req, err, -1, 0, 0);
      return (FAIL);
    }
  
  put_response (req, EP_OK, gid, 0, 0);
  return (SUCCESS);
}  
