/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/mount.c,v 1.2 1999/03/21 00:57:49 monaka Exp $ */

/*
 * $Log: mount.c,v $
 * Revision 1.2  1999/03/21 00:57:49  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.1  1997/08/31 13:10:46  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"

int
psys_mount (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_mount *args = (struct psc_mount *)argp;

  req.param.par_mount.devnamelen = args->devnamelen;
  req.param.par_mount.devname = args->devname;
  req.param.par_mount.dirnamelen = args->dirnamelen;
  req.param.par_mount.dirname = args->dirname;
  req.param.par_mount.fstypelen = args->fstypelen;
  req.param.par_mount.fstype = args->fstype;
  req.param.par_mount.option = args->option;

  error = _make_connection(PSC_MOUNT, &req, &res);

  if (error != E_OK) {
      /* What should I do? */
  }
  else if (res.errno) {
    ERRNO = res.errno;
    return (-1);
  }

  return (res.status);
}
