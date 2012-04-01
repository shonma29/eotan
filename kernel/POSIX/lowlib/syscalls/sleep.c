/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sleep.c,v 1.3 1999/03/21 00:58:18 monaka Exp $ */
static char rcsid[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/sleep.c,v 1.3 1999/03/21 00:58:18 monaka Exp $";

/*
 * $Log: sleep.c,v $
 * Revision 1.3  1999/03/21 00:58:18  monaka
 * Major fix. Many system calls runs correctly (...perhaps ;-). But signal and process functions are not yet.
 *
 * Revision 1.2  1999/02/19 08:59:44  monaka
 * added the parameter structure on.
 *
 * Revision 1.1  1997/08/31 13:10:50  night
 * 最初の登録
 *
 *
 *
 */


#include "../lowlib.h"

/* 実際の動作は usleep とする */
int
psys_sleep (void *argp)
{
  struct psc_sleep *args = (struct psc_sleep *)argp;
  W usecond;

  usecond = args->second;

  if (usecond < 0) {
    return(-1);
  }
  else if (usecond == 0) {
    return E_OK;
  }

  /* CLOCK への ROUNDUP */
  usecond = (usecond +  CLOCK - 1) / CLOCK;

  dly_tsk(usecond);

  return (E_OK);
}
