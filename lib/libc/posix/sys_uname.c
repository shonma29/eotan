/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_uname.c,v 1.1 1997/08/31 13:25:28 night Exp $  */

#include <sys/utsname.h>
#include "uname.h"

struct utsname		system_name = 
{
  SYS_SYSNAME,
  SYS_NODENAME,
  SYS_RELEASE,
  SYS_VERSION,
  SYS_MACHINE
};


/* uname 
 *
 */
int
uname (struct utsname *name)
{
  *name = system_name;

  return 0;
}
