/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/getdents.c,v 1.1 2000/06/01 08:45:29 naniwa Exp $ */

/*
 * $Log: getdents.c,v $
 * Revision 1.1  2000/06/01 08:45:29  naniwa
 * first version
 *
 *
 */


#include "../lowlib.h"

int
psys_getdents (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;
  struct psc_getdents *args = (struct psc_getdents *)argp;

  req.param.par_getdents.fileid = args->fileid;
  req.param.par_getdents.buf = args->buf;
  req.param.par_getdents.length = args->length;

  error = _make_connection(PSC_GETDENTS, &req);
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
