/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001, Tomohide Naniwa

*/

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/manager/request.c,v 1.7 2000/02/19 03:02:15 naniwa Exp $ */


/* request.c - POSIX 関係のメッセージ処理を行う
 *
 *
 *
 * $Log: request.c,v $
 * Revision 1.7  2000/02/19 03:02:15  naniwa
 * minor change on debug write
 *
 * Revision 1.6  1999/03/24 08:00:20  monaka
 * Modified include pathes (POSIX manager should be independ from ITRON.)
 *
 * Revision 1.5  1997/10/24 14:00:10  night
 * 変数の引数合わせを厳密にした。
 *
 * Revision 1.4  1997/04/28 15:29:10  night
 * デバイスドライバからのメッセージを受信する領域をスタック上に
 * とっていたところを、static 領域に設定した。
 *
 * Revision 1.3  1996/11/20  12:12:47  night
 * エラーチェックの追加。
 *
 * Revision 1.2  1996/11/07  12:48:52  night
 * get_request() と put_response() の追加。
 *
 * Revision 1.1  1996/11/05  15:13:46  night
 * 最初の登録
 *
 */
#include <core.h>
#include <nerve/kcall.h>
#include "fs.h"


/* put_response -
 *
 */
W
put_response(RDVNO rdvno, W error_no, W status, W ret1)
{
    struct posix_response res;
    ER syserr;

    res.error_no = error_no;
    res.status = status;
    res.ret1 = ret1;

    /* 要求元に送信する */
    syserr = kcall->port_reply(rdvno, &res, sizeof(res));
    return syserr? ECONNREFUSED:EOK;
}

W
put_response_long(RDVNO rdvno, W error_no, D status)
{
    struct posix_response res;
    ER syserr;
    D *dp = (D*)&(res.status);

    res.error_no = error_no;
    *dp = status;

    /* 要求元に送信する */
    syserr = kcall->port_reply(rdvno, &res, sizeof(res));
    return syserr? ECONNREFUSED:EOK;
}
