/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/umount.c,v 1.2 1999/03/21 00:58:26 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/umount.c,v 1.2 1999/03/21 00:58:26 monaka Exp $";

/*
 * $Log: umount.c,v $
 * Revision 1.2  1999/03/21 00:58:26  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.1  1997/08/31 13:10:51  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"

int
psys_umount (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_umount *args = (struct psc_umount *)argp;

  req.param.par_umount.dirnamelen = args->dirnamelen;
  req.param.par_umount.dirname = args->dirname;

  error = _make_connection(PSC_UMOUNT, &req, &res);

  if (error != E_OK) {
      /* What should I do? */
  }
  else if (res.errno) {
    ERRNO = res.errno;
    return (-1);
  }

  return (res.status);
}
