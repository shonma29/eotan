/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* POSIX Library misc function.
*/

/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/libc/native/sys_chown.c,v 1.1 1997/08/31 13:25:19 night Exp $  */

#include <string.h>
#include "../native.h"
#include "../errno.h"


/* chown 
 *
 */
int
chown (char *path, uid_t owner, gid_t group)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_chown.pathlen = strlen (path);
  req.param.par_chown.path = path;
  req.param.par_chown.uid = owner;
  req.param.par_chown.gid = group;

  error = _make_connection(PSC_CHOWN, &req);
  if (error != E_OK)
    {
      /* What should I do? */
    }

  else if (res->errno)
    {
      errno = res->errno;
      return (-1);
    }

  return (res->status);
}


