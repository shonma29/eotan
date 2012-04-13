/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/opendir.c,v 1.3 1999/03/21 00:57:53 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/opendir.c,v 1.3 1999/03/21 00:57:53 monaka Exp $";

/*
 * $Log: opendir.c,v $
 * Revision 1.3  1999/03/21 00:57:53  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:04:44  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:46  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_opendir (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_opendir *args = (struct psc_opendir *)argp;

  req.param.par_opendir.pathlen = args->pathlen;
  req.param.par_opendir.path = args->path;

  error = _make_connection(PSC_OPENDIR, &req, &res);
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

