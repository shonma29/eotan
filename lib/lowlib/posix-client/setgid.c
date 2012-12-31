/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/setgid.c,v 1.3 1999/03/21 00:58:03 monaka Exp $ */

/*
 * $Log: setgid.c,v $
 * Revision 1.3  1999/03/21 00:58:03  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 08:59:40  monaka
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
psys_setgid (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_setgid *args = (struct psc_setgid *)argp;

  req.param.par_setgid.gid = args->gid;

  error = _make_connection(PSC_SETGID, &req, &res);
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
