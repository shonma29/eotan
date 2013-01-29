/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/*
 * $Log: exec.c,v $
 * Revision 1.5  2000/02/27 15:35:30  naniwa
 * minor change
 *
 * Revision 1.4  2000/02/04 15:16:30  naniwa
 * minor fix
 *
 * Revision 1.3  1999/11/10 10:50:15  naniwa
 * implemented
 *
 * Revision 1.2  1999/03/24 03:54:51  monaka
 * printf() was renamed to printk().
 *
 * Revision 1.1  1999/03/21 14:01:51  monaka
 * They are separated from syscall.c. Still no major changes available.
 *
 */

#include "fs.h"

/* psc_exec_f - 指定されたプログラムファイルをメモリ中に読み込む
 */
W psc_exec_f(RDVNO rdvno, struct posix_request *req)
{
#ifdef USE_ALLOCA
    B *pathname;
#else
    B pathname[MAX_NAMELEN];
#endif
    W errno;

#ifdef EXEC_DEBUG
    printk("[PM] exec: start\n");
#endif
#ifdef USE_ALLOCA
    pathname = alloca(req->param.par_execve.pathlen + 1);
    if (pathname == NULL) {
	/* メモリ取得エラー */
	put_response(rdvno, EP_NOMEM, -1, 0);
	return (FAIL);
    }
#endif
#if 0
    bzero(pathname, req->param.par_execve.pathlen + 1);
#endif

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    errno = vget_reg(req->caller, req->param.par_execve.name,
		     req->param.par_execve.pathlen + 1, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EP_INVAL, -1, 0);
	else
	    put_response(rdvno, EP_FAULT, -1, 0);

	return (FAIL);
    }
#ifdef EXEC_DEBUG
    printk("exec: pathname is %s\n", pathname);
#endif
    errno = exec_program(req, req->procid, pathname);
    if (errno) {
	if (proc_get_vmtree(req->procid) != NULL) {
	    /* 呼び出しを行ったプロセスがまだ生き残っていた場合 */
	    /*エラーメッセージを返す */
	    put_response(rdvno, errno, -1, 0);
	} else {
	    /* 既にプロセスの仮想メモリが開放されている場合 */
	    /* exit が実行されることは無いので，ここで開放する */
	    proc_exit(req->procid);
	}
	return (FAIL);
    }

    /* プロセスに属するタスクを切り換える。
     * すなわち、古いタスクを終了し、新しいタスクを生成する。
     */


    /* exec システムコールは、成功すると元のプロセスは消えるので、
     * レスポンスを返さない
     */
    return (SUCCESS);
}
