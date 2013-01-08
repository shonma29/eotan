/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/dup2.c,v 1.3 1999/03/21 00:57:18 monaka Exp $ */

/*
 * $Log: dup2.c,v $
 * Revision 1.3  1999/03/21 00:57:18  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 08:59:14  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:43  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_dup2 (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;
  struct psc_dup2 *args = (struct psc_dup2 *)argp;

  req.param.par_dup2.fileid1 = args->fileid1;
  req.param.par_dup2.fileid2 = args->fileid2;

  error = _make_connection(PSC_DUP2, &req);
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
