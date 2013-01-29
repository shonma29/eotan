/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: statfs.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

W
psc_statfs_f (RDVNO rdvno, struct posix_request *req)
{
  struct statfs	result;
  ER		errno;

  errno = fs_statfs (req->param.par_statfs.device, &result);
  if (errno)
    {
      put_response (rdvno, req, errno, -1, 0, 0);    
      return (FAIL);
    }

#ifdef notdef
  printf ("result.f_type = 0x%x\n", result.f_type);
  printf ("result.f_blksize = 0x%x\n", result.f_bsize);
#endif
  
  errno = vput_reg (req->caller, req->param.par_statfs.fsp, sizeof (struct statfs), &result);
  if (errno)
    {
      put_response (rdvno, req, EP_FAULT, -1, 0, 0);
      return (FAIL);
    }
  put_response (rdvno, req, EP_OK, 0, 0, 0);
  return (SUCCESS);
}
