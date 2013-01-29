/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: mountroot.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_mountroot_f (RDVNO rdvno, struct posix_request *req)
{
  W	err;

#ifdef notdef
  printf ("mountroot: start (device = %d )\n", req->param.par_mountroot.device);
#endif  
  err = mount_root (req->param.par_mountroot.device, 
		    req->param.par_mountroot.fstype,
		    req->param.par_mountroot.option);
#ifdef notdef
  printf ("mountroot: called mount_root(), err = %d\n", err);
#endif  

  if (err)
    {
      put_response (rdvno, err, -1, 0);
      return (FAIL);
    }
  else
    {
      put_response (rdvno, err, 0, 0);
    }
  return (SUCCESS);
}
