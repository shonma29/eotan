/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/getegid.c,v 1.3 1999/03/21 00:57:31 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/getegid.c,v 1.3 1999/03/21 00:57:31 monaka Exp $";

/*
 * $Log: getegid.c,v $
 * Revision 1.3  1999/03/21 00:57:31  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 10:04:35  monaka
 * Addition or fix for parameter structure.
 *
 * Revision 1.1  1997/08/31 13:10:44  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_getegid (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
#if 0
  struct psc_getegid
    {
      /* void */
    } *args = (struct psc_getegid *)argp;
#endif

  error = _make_connection(PSC_GETEGID, &req, &res);
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
