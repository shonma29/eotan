/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_sleep.c,v 1.1 1997/08/31 13:25:27 night Exp $  */

#include <config.h>
#include "posix.h"


/* usleep --- システムコールの動作は usleep とする．
 *
 */
int
usleep (int usecond)
{
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

/* sleep 
 *
 */
int
sleep (int second)
{
  return usleep(second * 1000);
}
