/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/stat.c,v 1.3 1999/03/21 00:58:19 monaka Exp $ */

/*
 * $Log: stat.c,v $
 * Revision 1.3  1999/03/21 00:58:19  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/03/16 16:26:38  monaka
 * Added some code from BTRON/init (but it runs under POSIX). May be these code will work but I don't test well.
 *
 * Revision 1.1  1997/08/31 13:10:50  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_fstat (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response *res = (struct posix_response*)&req;
  struct psc_fstat *args = (struct psc_fstat *)argp;

  req.param.par_fstat.fileid = args->fileid;
  req.param.par_fstat.st = args->st;

  error = _make_connection(PSC_FSTAT, &req);
  if (error != E_OK)
    {
      /* What should I do? */
    }

  else if (res->errno)
    {
      ERRNO = res->errno;
      return (-1);
    }

  return (res->status);
}
