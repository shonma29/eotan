/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/remove.c,v 1.4 1999/03/21 00:57:58 monaka Exp $ */

/*
 * $Log: remove.c,v $
 * Revision 1.4  1999/03/21 00:57:58  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.3  1999/02/19 10:04:48  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.2  1999/02/19 08:59:36  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:47  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_remove (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_remove *args = (struct psc_remove *)argp;

  req.param.par_remove.pathlen = args->pathlen;
  req.param.par_access.path = args->path;

  error = _make_connection(PSC_REMOVE, &req, &res);
  if (error != E_OK)
    {
      /* What should I do? */
    }

  else if (res.errno)
    {
      ERRNO = res.errno;
      return (-1);
    }

  return (res.status);
}
