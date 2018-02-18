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

#ifdef EXEC_DEBUG
    dbg_printf("fs: exec: start\n");
#endif

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = kcall->region_copy(get_rdv_tid(req->rdvno), req->packet.param.par_execve.name,
		     sizeof(req->buf) - 1, req->buf);
    if (error_no < 0) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    return EINVAL;
	else
	    return EFAULT;
    }
    req->buf[sizeof(req->buf) - 1] = '\0';
#ifdef EXEC_DEBUG
    dbg_printf("fs: exec: pathname is %s\n", req->buf);
#endif
    error_no = exec_program(&(req->packet), req->packet.procid, req->buf);
    if (error_no) {
	if (proc_get_status(req->packet.procid) == PS_RUN) {
	    /* 呼び出しを行ったプロセスがまだ生き残っていた場合 */
	    /*エラーメッセージを返す */
	    return error_no;
	} else {
	    /* 既にプロセスの仮想メモリが開放されている場合 */
	    /* exit が実行されることは無いので，ここで開放する */
	    proc_exit(req->packet.procid);
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
  struct proc *myprocp, *procp;
  W mypid, wpid, exst;
  W i;
  ER error_no;
  ID tskid;

  mypid = req->packet.procid;
  error_no = proc_get_procp(mypid, &myprocp);
  if (error_no)
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return ESRCH;

  myprocp->proc_exst = req->packet.param.par_exit.evalue;

  error_no = proc_get_procp(myprocp->proc_ppid, &procp);
  if (error_no)
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return ESRCH;

  wpid = procp->proc_wpid;
  if (procp->proc_status == PS_WAIT &&
      (wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
    /* 親プロセスが自分を WAIT していればメッセージ送信 */
    procp->proc_status = PS_RUN;
    exst = (myprocp->proc_exst << 8);
    reply2(procp->proc_wait_rdvno, 0, mypid, exst);

    /* エントリーの開放 */
    proc_exit(mypid);
  }
  else {
    /* そうでなければ，ZOMBIE 状態に */
    myprocp->proc_status = PS_ZOMBIE;
  }

  /* 子プロセスの親を INIT に変更 */
  for(i = INIT_PID + 1; i < MAX_PROCESS; ++i) {
    proc_get_procp(i, &procp);
    if (procp->proc_status == PS_DORMANT) continue;
    if (procp->proc_ppid != mypid) continue;
    procp->proc_ppid = INIT_PID; /* INIT プロセスの pid は 0 */
    kcall->region_put(procp->proc_maintask,
	     (pid_t*)(LOCAL_ADDR + offsetof(thread_local_t, parent_process_id)),
	     sizeof(pid_t), &(procp->proc_ppid));
    
    /* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
  }

  tskid = get_rdv_tid(req->rdvno);
  kcall->thread_terminate(tskid);
  kcall->thread_destroy(tskid);

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

  error_no = proc_get_procp (req->packet.procid, &procp);		/* 親プロセスの情報の取りだし */
  if (error_no)
    {
      dbg_printf ("fs: invalid process id (%d)\n", req->packet.procid);
      return error_no;
    }

#ifdef DEBUG
  dbg_print ("fs: if_fork(): proc = 0x%x\n", procp);
#endif

  error_no = proc_alloc_proc(&child);
  if (error_no)
    {
      dbg_printf ("fs: cannot allocate process\n");
      return error_no;
    }

  error_no = proc_fork (procp, child);
  if (error_no)
    {
      proc_dealloc_proc(child->proc_pid);
      return error_no;
    }

  main_thread_id = thread_create(child->proc_pid, req->packet.param.par_fork.entry,
      (VP)(req->packet.param.par_fork.sp));
  if (main_thread_id < 0)
    {
      dbg_printf ("fs: acre_tsk error (%d)\n", main_thread_id);
      proc_dealloc_proc(child->proc_pid);
      return error_no;
    }

  child->proc_maintask = main_thread_id;

  error_no = copy_local(procp, child);
  if (error_no)
    {
      proc_dealloc_proc(child->proc_pid);
      kcall->thread_destroy(main_thread_id);
//TODO destroy process
      return error_no;
    }

  kcall->thread_start(main_thread_id);

  reply2(req->rdvno, 0, child->proc_pid, 0);	/* 親プロセスに対して応答 */
  return EOK;
}  

int if_kill(fs_request *req)
{
    struct proc *myprocp, *procp;
    W mypid, wpid, exst;
    W i;
    ER error_no;

    /* req->caller が task 1 の場合は，返事のメッセージを送らない */

    mypid = req->packet.args.arg1;
    error_no = proc_get_procp(mypid, &myprocp);
    if (error_no)
	return ESRCH;

    myprocp->proc_exst = (-1);	/* 強制終了時のステータスは (-1) で良いか? */

    error_no = proc_get_procp(myprocp->proc_ppid, &procp);
    if (error_no)
	return ESRCH;

    wpid = procp->proc_wpid;
    if (procp->proc_status == PS_WAIT &&
	(wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
	/* 親プロセスが自分を WAIT していればメッセージ送信 */
	procp->proc_status = PS_RUN;
	exst = (myprocp->proc_exst << 8);
	reply2(req->rdvno, 0, mypid, exst);

	/* エントリーの開放 */
	proc_exit(mypid);
    } else {
	/* そうでなければ，ZOMBIE 状態に */
	myprocp->proc_status = PS_ZOMBIE;
    }

    /* 子プロセスの親を INIT に変更 */
    for (i = INIT_PID + 1; i < MAX_PROCESS; ++i) {
	proc_get_procp(i, &procp);
	if (procp->proc_status == PS_DORMANT)
	    continue;
	if (procp->proc_ppid != mypid)
	    continue;
	procp->proc_ppid = INIT_PID;	/* INIT プロセスの pid は 0 */
	kcall->region_put(procp->proc_maintask,
		(pid_t*)(LOCAL_ADDR + offsetof(thread_local_t, parent_process_id)),
		sizeof(pid_t), &(procp->proc_ppid));

	/* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
    }

    /* メインタスクの強制終了 */
    kcall->thread_terminate(myprocp->proc_maintask);
    kcall->thread_destroy(myprocp->proc_maintask);

    if (get_rdv_tid(req->rdvno) != myprocp->proc_maintask) {
	reply2(req->rdvno, 0, 0, 0);
    }

    return EOK;
}

int
if_waitpid (fs_request *req)
{
  W i;
  W mypid, pid, children, exst;
  struct proc *procp;

  pid = req->packet.param.par_waitpid.pid;
  mypid = req->packet.procid;
  if (pid == 0) pid = (-proc_table[mypid].proc_pgid);

  /* プロセステーブルを走査して子プロセスを調査 */
  children = 0;
  for(i = INIT_PID + 1; i < MAX_PROCESS; ++i) {
    proc_get_procp(i, &procp);
    if (procp->proc_status == PS_DORMANT) continue;
    if (procp->proc_ppid == mypid) {
      if (pid > 0 && pid != procp->proc_pid) continue;
      if (pid < -1 && pid != -procp->proc_pgid) continue;
      children++;
      if (procp->proc_status == PS_ZOMBIE) {
	/* 子プロセスの情報をクリアし，親プロセスに返事を送る */
	exst = (procp->proc_exst << 8);
	reply2(req->rdvno, 0, i, exst);
	
	/* 親プロセスの状態変更 */
	proc_get_procp(mypid, &procp);
	procp->proc_status = PS_RUN;

	/* 子プロセスのエントリーの開放 */
	proc_exit(i);
	
	return EOK;
      }
    }
  }
  if (children > 0) {
    /* 対応する子プロセスはあったが，まだ終了していなかった */
    if (req->packet.param.par_waitpid.opts & WNOHANG) {
      /* 親に返事を送る必要がある */
      reply2(req->rdvno, 0, 0, 0);
    }
    /* 親プロセスの状態を変更し，返事を送らずにシステムコールを終了 */
    proc_get_procp(mypid, &procp);
    procp->proc_status = PS_WAIT;
    procp->proc_wpid = pid;
    procp->proc_wait_rdvno = req->rdvno;
  }
  else
    /* エラーを返す */
    return ECHILD;

  return EOK;
}  
