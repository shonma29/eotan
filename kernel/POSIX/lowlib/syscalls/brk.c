/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header$ */
static char rcsid[] = "@(#)$Header$";

/*
 * $Log$
 *
 */


#include "../lowlib.h"

int
psys_brk (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response res;
  struct psc_brk *args = (struct psc_brk *)argp;

  req.param.par_brk.end_adr = args->end_adr;

  error = _make_connection(PSC_BRK, &req, &res);
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
