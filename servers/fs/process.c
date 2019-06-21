/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2003, Tomohide Naniwa

*/
/*
 * $Log$
 *
 */

#include <core.h>
#include <local.h>
#include <stddef.h>
#include <thread.h>
#include <mm/segment.h>
#include <boot/init.h>
#include <core/options.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <sys/errno.h>
#include <sys/syslimits.h>
#include <sys/wait.h>
#include "../../lib/libserv/libmm.h"
#include "api.h"
#include "procfs/process.h"
#include "../../lib/libserv/libserv.h"

/* if_exec - 指定されたプログラムファイルをメモリ中に読み込む
 */
int if_exec(fs_request *req)
{
    W error_no;

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = kcall->region_copy(unpack_tid(req), (void*)(req->packet.arg1),
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[sizeof(req->buf) - 1] = '\0';
    error_no = exec_program(&(req->packet), unpack_pid(req), req->buf);
    if (error_no) {
	if (proc_get_status(unpack_pid(req)) == PS_RUN) {
	    /* 呼び出しを行ったプロセスがまだ生き残っていた場合 */
	    /*エラーメッセージを返す */
	    return error_no;
	} else {
	    /* 既にプロセスの仮想メモリが開放されている場合 */
	    /* exit が実行されることは無いので，ここで開放する */
	    proc_exit(unpack_pid(req));
	    return EOK;
	}
    }
//TODO check if rdvno will be disposed
    reply2(req->rdvno, 0, 0, 0);
    return EOK;
}

/* if_exit - プロセスを終了させる
 */
int
if_exit (fs_request *req)
{
  struct proc *myprocp;
  W mypid;
  ER error_no;

  mypid = unpack_pid(req);
  error_no = proc_get_procp(mypid, &myprocp);
  if (error_no)
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return ESRCH;

  /* エントリーの開放 */
  proc_exit(mypid);

  reply2(req->rdvno, 0, 0, 0);
  return EOK;
}  

/* if_fork - 新しいプロセスを作成する
 */
int
if_fork (fs_request *req)
{
  struct proc *procp;
  W	       error_no;
  ID main_thread_id;
  struct proc *child;

  error_no = proc_get_procp (unpack_pid(req), &procp);		/* 親プロセスの情報の取りだし */
  if (error_no)
    {
      log_debug("fs: invalid process id (%d)\n", req->packet.procid);
      return error_no;
    }

  error_no = proc_alloc_proc(&child);
  if (error_no)
    {
      log_debug("fs: cannot allocate process\n");
      return error_no;
    }

  error_no = proc_fork (procp, child);
  if (error_no)
    {
      proc_dealloc_proc(child->proc_pid);
      return error_no;
    }

  main_thread_id = thread_create(child->proc_pid, (FP)(req->packet.arg2),
      (VP)(req->packet.arg1));
  if (main_thread_id < 0)
    {
      log_debug("fs: create error=%d\n", main_thread_id);
      proc_dealloc_proc(child->proc_pid);
      return error_no;
    }

  kcall->thread_start(main_thread_id);

  reply2(req->rdvno, 0, child->proc_pid, 0);	/* 親プロセスに対して応答 */
  return EOK;
}  
