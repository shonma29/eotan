/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/execle.c,v 1.2 1999/03/21 00:57:21 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/execle.c,v 1.2 1999/03/21 00:57:21 monaka Exp $";

/*
 * $Log: execle.c,v $
 * Revision 1.2  1999/03/21 00:57:21  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.1  1997/08/31 13:10:43  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"




int
psys_execle (void *argp)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	res;
  struct psc_execle *args = (struct psc_execle *)argp;

  /*
   * まだインプリメントしていない。ENOSYS を返す。
   */
  errno = ENOSYS;

  return (NULL);
}
