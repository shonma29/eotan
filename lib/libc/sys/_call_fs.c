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
#include <local.h>
#include <services.h>
#include <core.h>
#include "sys.h"


ER
_make_connection(W wOperation,
		 pm_args_t *req)
{
    thread_local_t *local_data = _get_local();
    W rsize;

    req->operation = wOperation;
    req->process_id = local_data->process_id;

    ID port;
    switch (wOperation) {
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
	port = PORT_PM;
	break;
    default:
	port = PORT_FS;
	break;
    }

    rsize = cal_por(port, 0xffffffff, req, sizeof(*req));

    if (rsize < 0) {
	return (ECONNREFUSED);
    }

    return (E_OK);
}

W
_call_fs(W wOperation, pm_args_t *req)
{
  pm_reply_t *res = (pm_reply_t*)req;
  ER error = _make_connection(wOperation, req);
  thread_local_t *local_data = _get_local();

  if (error != E_OK)
    {
      local_data->error_no = error;
      return (-1);
    }

  else if (res->error_no)
    {
      local_data->error_no = res->error_no;
      return (-1);
    }

  return (res->result1);
}
