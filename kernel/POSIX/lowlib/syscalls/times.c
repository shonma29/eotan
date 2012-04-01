/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/times.c,v 1.3 1999/03/21 00:58:22 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/times.c,v 1.3 1999/03/21 00:58:22 monaka Exp $";

/*
 * $Log: times.c,v $
 * Revision 1.3  1999/03/21 00:58:22  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:05:06  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:50  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_times (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_times *args = (struct psc_times *)argp;

  req.param.par_times.buf = args->buf;

  error = _make_connection(PSC_TIMES, &req, &res);
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
