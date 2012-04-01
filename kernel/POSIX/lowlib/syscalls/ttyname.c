/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/ttyname.c,v 1.4 1999/03/21 00:58:23 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/ttyname.c,v 1.4 1999/03/21 00:58:23 monaka Exp $";

/*
 * $Log: ttyname.c,v $
 * Revision 1.4  1999/03/21 00:58:23  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.3  1999/02/19 10:05:08  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.2  1999/02/19 08:59:45  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:50  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_ttyname (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_ttyname *args = (struct psc_ttyname *)argp;

  req.param.par_ttyname.namelen = args->namelen;
  req.param.par_ttyname.name = args->name;

  error = _make_connection(PSC_TTYNAME, &req, &res);
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
