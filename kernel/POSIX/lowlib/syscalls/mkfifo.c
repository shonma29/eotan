/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/mkfifo.c,v 1.4 1999/03/21 00:57:48 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/mkfifo.c,v 1.4 1999/03/21 00:57:48 monaka Exp $";

/*
 * $Log: mkfifo.c,v $
 * Revision 1.4  1999/03/21 00:57:48  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.3  1999/02/19 10:04:41  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.2  1999/02/19 08:59:34  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:46  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_mkfifo (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_mkfifo *args = (struct psc_mkfifo *)argp;

  req.param.par_mkfifo.pathlen = args->pathlen;
  req.param.par_mkfifo.path = args->path;
  req.param.par_mkfifo.mode = args->mode;

  error = _make_connection(PSC_MKFIFO, &req, &res);
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
