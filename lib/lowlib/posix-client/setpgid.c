/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/setpgid.c,v 1.3 1999/03/21 00:58:04 monaka Exp $ */

/*
 * $Log: setpgid.c,v $
 * Revision 1.3  1999/03/21 00:58:04  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 08:59:41  monaka
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
psys_setpgid (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;
  struct psc_setpgid *args = (struct psc_setpgid *)argp;

  req.param.par_setpgid.gid = args->gid;

  error = _make_connection(PSC_SETPGID, &req);
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
