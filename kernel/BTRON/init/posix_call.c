/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/init/posix_syscall.c,v 1.2 1999/03/16 16:31:53 monaka Exp $ */

static char rcsid[] = "$Id: posix_syscall.c,v 1.2 1999/03/16 16:31:53 monaka Exp $";


#include "init.h"

W
sys_noaction (void)
{
  return call_posix (0, NULL);
}
