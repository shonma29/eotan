/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/fcntl.c,v 1.3 1999/03/21 00:57:27 monaka Exp $ */

/*
 * $Log: fcntl.c,v $
 * Revision 1.3  1999/03/21 00:57:27  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 08:59:16  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:44  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_fcntl (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;
  struct psc_fcntl *args = (struct psc_fcntl *)argp;

  req.param.par_fcntl.fileid = args->fileid;
  req.param.par_fcntl.cmd = args->cmd;
  req.param.par_fcntl.arg = args->arg;

  error = _make_connection(PSC_FCNTL, &req);
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
