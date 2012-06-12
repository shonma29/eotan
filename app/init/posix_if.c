/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

/* $Id: posix_if.c,v 1.20 2000/02/16 08:20:36 naniwa Exp $ */

static char rcsid[] =
    "$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/init/posix_if.c,v 1.20 2000/02/16 08:20:36 naniwa Exp $";


/* 
 * $Log: posix_if.c,v $
 * Revision 1.20  2000/02/16 08:20:36  naniwa
 * to reorder POSIX system calls
 *
 * Revision 1.19  2000/01/28 10:28:03  monaka
 * Some hooks for POSIX manager (especially file functions) was deleted.
 * Now Init is more close to POSIX libs.
 *
 * Revision 1.18  1999/11/10 10:23:58  naniwa
 * to support execve
 *
 * Revision 1.17  1999/07/26 08:59:09  naniwa
 * minor chage of posix_fork()
 *
 * Revision 1.16  1999/07/24 04:35:07  naniwa
 * minor change
 *
 * Revision 1.15  1999/07/09 08:25:20  naniwa
 * minor fix
 *
 * Revision 1.14  1999/04/13 04:14:45  monaka
 * MAJOR FIXcvs commit -m 'MAJOR FIX!!! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.'! There are so many changes, modifys, fixes. Sorry but I can't remember all of those. For example, all the manager and driver programmer have got power to access all ITRON systemcall. (My works is just making access route to ITRON. I don't know what happens in the nuclus.
 *
 * Revision 1.13  1999/03/16 16:22:14  monaka
 * File operators was modified to use POSIX lowlib.
 *
 * Revision 1.12  1999/03/15 08:43:25  monaka
 * Minor fix.
 *
 * Revision 1.11  1999/02/17 09:43:10  monaka
 * Modified for 'device descriptor'.
 *
 * Revision 1.10  1998/05/23 15:30:54  night
 * POSIX サーバへの呼び出しコマンドを READ から READDIR に変更。
 *
 * Revision 1.9  1998/02/24 14:05:52  night
 * posix_fork() の修正。
 * デバッグ用のプリント文の追加。
 * 子プロセスのタスクのスタックサイズを 10KB から 12 KB に
 * 変更(ページ境界に合わせた)。
 *
 * Revision 1.8  1998/02/16 14:12:52  night
 * posix_fork () 関数の追加。
 * この関数は、fork システムコールの処理のテストを行うための
 * 関数。
 * コマンド pfork を実行するときに呼び出される。
 *
 * Revision 1.7  1998/01/06 16:39:18  night
 * レクエストパケットにコマンド情報が入っていなかったので、追加した。
 *
 * Revision 1.5  1997/10/11 16:20:15  night
 * ファイルの書き込み処理を lowlib を介さないように変更。
 *
 * Revision 1.4  1997/09/09 13:49:30  night
 * posix_write 関数を追加。
 *
 * Revision 1.3  1997/08/31 14:05:06  night
 * BOOT が HD から行われた時には、自動的に POSIX の root file system を
 * 設定するように処理を変更した。
 *
 * Revision 1.2  1997/07/09 15:00:42  night
 * 文字コードを EUC に変更。
 * posix_access() と posix_newprocess() の追加。
 *
 * Revision 1.1  1997/07/07 12:18:56  night
 * 最初の登録
 *
 *
 */

#include "../../include/itron/struct.h"
#include "../../include/services.h"
#include "../../servers/fs/posix_sysc.h"
#include "../../servers/fs/posix_mm.h"
#include "init.h"

static ID recv_port;
static ID myself;

static W posix_mountroot(W root_device);

/*
 *
 */
W posix_init(W root_device)
{
    ER error;
    struct posix_request req;
    struct posix_response res;
    INT rsize;
    T_CMBF pk_cmbf = { NULL, TA_TFIFO, 0, sizeof(res) };

    recv_port = acre_mbf(&pk_cmbf);
    if (recv_port == 0) {
	printf("Cannot allocate port\n");
	return (E_NOMEM);
    }

    if (get_tid(&myself) != E_OK) {
	printf("Can not get own taskid.\n");
	return (E_SYS);
    }
#ifdef DEBUG
    printf("init: pinit, send port = %d, receive port = %d\n",
	   PORT_FS, recv_port);
#endif

    /* ROOT ファイルシステムの設定
     */
    error = posix_mountroot(root_device);
    if (error) {
	return (E_SYS);
    }

    /* init プロセスの情報の設定
     */
    req.receive_port = recv_port;
    req.msg_length = sizeof(res);
    req.operation = PSC_MISC;
    req.procid = 0;
    req.caller = myself;
    req.param.par_misc.cmd = M_SET_PROCINFO;
    req.param.par_misc.arg.set_procinfo.proc_maintask = myself;
    req.param.par_misc.arg.set_procinfo.proc_signal_handler = NULL;
    req.param.par_misc.arg.set_procinfo.proc_uid = 0;
    req.param.par_misc.arg.set_procinfo.proc_gid = 0;
    req.param.par_misc.arg.set_procinfo.proc_euid = 0;
    req.param.par_misc.arg.set_procinfo.proc_egid = 0;
    req.param.par_misc.arg.set_procinfo.proc_umask = 022;
    req.param.par_misc.arg.set_procinfo.proc_pid = 0;
    req.param.par_misc.arg.set_procinfo.proc_ppid = 0;
    req.param.par_misc.arg.set_procinfo.proc_access =
	VM_READ | VM_WRITE | VM_EXEC;
    snd_mbf(PORT_FS, sizeof(req), &req);

    rsize = sizeof(res);
    rcv_mbf(&res, &rsize, recv_port);

    return (E_OK);
}


static W posix_mountroot(W root_device)
{
    struct posix_request req;
    struct posix_response res;
    INT rsize;

    req.receive_port = recv_port;
    req.msg_length = sizeof(req);
    req.operation = PSC_MOUNTROOT;
    req.param.par_mountroot.device = root_device;
    req.param.par_mountroot.fstype = 1;
    req.param.par_mountroot.option = 0;
    snd_mbf(PORT_FS, sizeof(req), &req);
    rsize = sizeof(res);
    rcv_mbf(&res, &rsize, recv_port);
    if (res.errno) {
	printf("syscall error = %d\n", (int) res.errno);
    } else {
	printf("syscall success.\n");
    }

    return (E_OK);
}
