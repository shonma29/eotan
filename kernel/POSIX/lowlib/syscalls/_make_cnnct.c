/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/_make_connection.c,v 1.3 2000/02/16 08:17:44 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/lowlib/syscalls/_make_connection.c,v 1.3 2000/02/16 08:17:44 naniwa Exp $";

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

#include "../lowlib.h"

ER
_make_connection(W wOperation,
		 struct posix_request *req, struct posix_response *res)
{
    ER error;
    W rsize;
    ID myself;

    if (get_tid(&myself)) {
	return (EP_PERM);
    }

    req->receive_port = lowlib_data->recv_port;
    req->msg_length = sizeof(struct posix_request);
    req->operation = wOperation;
    req->procid = MY_PID;
    req->caller = myself;
    snd_mbf(posix_manager, sizeof(struct posix_request), req);

    rsize = sizeof(struct posix_response);
    error = rcv_mbf(res, (INT *) & rsize, lowlib_data->recv_port);
    if (error) {
	return (EP_PERM);
    }

    return (E_OK);
}
