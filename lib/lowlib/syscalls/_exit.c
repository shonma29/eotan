/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/_exit.c,v 1.4 2000/02/16 08:17:43 naniwa Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/_exit.c,v 1.4 2000/02/16 08:17:43 naniwa Exp $";

/*
 * $Log: _exit.c,v $
 * Revision 1.4  2000/02/16 08:17:43  naniwa
 * minor fix
 *
 * Revision 1.3  1999/03/21 00:57:08  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:04:26  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:42  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys__exit (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_exit *args = (struct psc_exit *)argp;

  req.param.par_exit.evalue = args->evalue;
  error = _make_connection(PSC_EXIT, &req, &res);
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