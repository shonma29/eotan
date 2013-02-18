/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/_make_connection.c,v 1.3 2000/02/16 08:17:44 naniwa Exp $ */

/*
 * $Log: _make_connection.c,v $
 * Revision 1.3  2000/02/16 08:17:44  naniwa
 * minor fix
 *
 * Revision 1.2  1999/03/30 13:25:22  monaka
 * Minor fixes.
 *
 * Revision 1.1  1999/03/21 01:00:25  monaka
 * First version.
 *
 */
#include <errno.h>
#include <services.h>
#include <itron/rendezvous.h>
#include "posix.h"


ER
_make_connection(W wOperation,
		 struct posix_request *req)
{
    struct lowlib_data	*lowlib_data = (struct lowlib_data *)LOWLIB_DATA;
    W rsize;
    ID myself;

    if (get_tid(&myself)) {
	return (EPERM);
    }

    req->msg_length = sizeof(struct posix_request);
    req->operation = wOperation;
    req->procid = lowlib_data->my_pid;
    req->caller = myself;
    rsize = cal_por(PORT_FS, 0xffffffff, req, sizeof(struct posix_request));

    if (rsize < 0) {
	return (EPERM);
    }

    return (E_OK);
}

W
_call_fs(W wOperation, struct posix_request *req)
{
  struct posix_response *res = (struct posix_response*)req;
  ER error = _make_connection(wOperation, req);

  if (error != E_OK)
    {
      /* What should I do? */
      errno = ESVC;
      return (-1);
    }

  else if (res->errno)
    {
      errno = res->errno;
      return (-1);
    }

  return (res->status);
}
