/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 * $Log: rewind.c,v $
 * Revision 1.1  1999/03/21 14:01:52  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "posix.h"

W
psc_rewind_f (RDVNO rdvno, struct posix_request *req)
{
  W	err;


  err = proc_rewind_file (req->procid, req->param.par_rewind.fileid);
  if (err)
    {
      put_response (rdvno, req, err, -1, 0, 0);
      return (FAIL);
    }

  put_response (rdvno, req, EP_OK, 0, 0, 0);
  return (SUCCESS);
}  
