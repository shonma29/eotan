/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/readdir.c,v 1.4 1999/12/26 11:15:04 naniwa Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/readdir.c,v 1.4 1999/12/26 11:15:04 naniwa Exp $";

/*
 * $Log: readdir.c,v $
 * Revision 1.4  1999/12/26 11:15:04  naniwa
 * implemented
 *
 * Revision 1.3  1999/03/21 00:57:57  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:04:47  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:47  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_readdir (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_readdir *args = (struct psc_readdir *)argp;

  req.param.par_readdir.fileid = args->fileid;
  req.param.par_readdir.buf = args->buf;
  req.param.par_readdir.length = args->length;

  error = _make_connection(PSC_READDIR, &req, &res);
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
