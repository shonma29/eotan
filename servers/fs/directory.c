/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: getdents.c,v $
 * Revision 1.1  2000/06/01 08:48:06  naniwa
 * first version
 *
 *
 */

#include <core/options.h>
#include <nerve/kcall.h>
#include <sys/syslimits.h>
#include "fs.h"
#include "api.h"

int if_link(fs_request *req)
{
    B src[NAME_MAX + 1];
    struct permission acc;
    W error_no;
    ID caller = get_rdv_tid(req->rdvno);

    error_no = kcall->region_copy(caller, (UB*)(req->packet.args.arg1),
		     sizeof(src) - 1, src);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    src[NAME_MAX] = '\0';
    error_no = kcall->region_copy(caller, (UB*)(req->packet.args.arg2),
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[NAME_MAX] = '\0';

    /* プロセスのユーザ ID とグループ ID の
     * 取り出し。
     * この情報に基づいて、ファイルを削除できるかどうかを
     * 決定する。
     */
    error_no = proc_get_permission(req->packet.procid, &acc);
    if (error_no)
	return error_no;

    error_no = fs_link_file(req->packet.procid, src, req->buf, &acc);
    if (error_no)
	return error_no;

    put_response(req->rdvno, EOK, 0, 0);
    return EOK;
}
