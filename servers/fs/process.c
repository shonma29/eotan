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
#include <thread.h>
#include <mm/segment.h>
#include <boot/init.h>
#include <core/options.h>
#include <mpu/memory.h>
#include <nerve/config.h>
#include <nerve/kcall.h>
#include <sys/wait.h>
#include "../../lib/libserv/libmm.h"
#include "fs.h"
#include "api.h"

/* if_exec - 指定されたプログラムファイルをメモリ中に読み込む
 */
void if_exec(fs_request *req)
{
    W error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

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
	    put_response(req->rdvno, EINVAL, -1, 0);
	else
	    put_response(req->rdvno, EFAULT, -1, 0);

	return;
    }
    req->buf[MAX_NAMELEN] = '\0';
#ifdef EXEC_DEBUG
    dbg_printf("fs: exec: pathname is %s\n", req->buf);
#endif
    error_no = exec_program(&(req->packet), req->packet.procid, req->buf);
    if (error_no) {
//TODO check by another way
	if (proc_get_vmtree(req->packet.procid) != NULL) {
	    /* 呼び出しを行ったプロセスがまだ生き残っていた場合 */
	    /*エラーメッセージを返す */
	    put_response(req->rdvno, error_no, -1, 0);
	} else {
	    /* 既にプロセスの仮想メモリが開放されている場合 */
	    /* exit が実行されることは無いので，ここで開放する */
	    proc_exit(req->packet.procid);
	}
	return;
    }
//TODO check if rdvno will be disposed
    put_response(req->rdvno, EOK, 0, 0);
}

/* if_exit - プロセスを終了させる
 */
void
if_exit (fs_request *req)
{
  struct proc *myprocp, *procp;
  W mypid, wpid, exst;
  W i;
  ER error_no;
  ID tskid;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  mypid = req->packet.procid;
  error_no = proc_get_procp(mypid, &myprocp);
  if (error_no) {
    put_response (req->rdvno, ESRCH, 0, 0);
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return;
  }

  myprocp->proc_exst = req->packet.param.par_exit.evalue;

  error_no = proc_get_procp(myprocp->proc_ppid, &procp);
  if (error_no) {
    put_response (req->rdvno, ESRCH, 0, 0);
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return;
  }

  wpid = procp->proc_wpid;
  if (procp->proc_status == PS_WAIT &&
      (wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
    /* 親プロセスが自分を WAIT していればメッセージ送信 */
    procp->proc_status = PS_RUN;
    exst = (myprocp->proc_exst << 8);
    put_response (procp->proc_wait_rdvno, EOK, mypid, exst);

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

  put_response (req->rdvno, EOK, 0, 0);
}  

/* if_fork - 新しいプロセスを作成する
 */
void
if_fork (fs_request *req)
{
  struct proc *procp;
  W	       error_no;
  ID main_thread_id;
  struct proc *child;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  error_no = proc_get_procp (req->packet.procid, &procp);		/* 親プロセスの情報の取りだし */
  if (error_no)
    {
      dbg_printf ("fs: invalid process id (%d)\n", req->packet.procid);
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

#ifdef DEBUG
  dbg_print ("fs: if_fork(): proc = 0x%x, proc->vm_tree = 0x%x\n", procp, procp->vm_tree);
#endif

  error_no = proc_alloc_proc(&child);
  if (error_no)
    {
      dbg_printf ("fs: cannot allocate process\n");
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

  error_no = proc_fork (procp, child);
  if (error_no)
    {
      proc_dealloc_proc(child->proc_pid);
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

  main_thread_id = thread_create(child->proc_pid, req->packet.param.par_fork.entry,
      (VP)(req->packet.param.par_fork.sp));
  if (main_thread_id < 0)
    {
      dbg_printf ("fs: acre_tsk error (%d)\n", main_thread_id);
      proc_dealloc_proc(child->proc_pid);
      put_response (req->rdvno, error_no, -1, 0);
      return;
    }

  child->proc_maintask = main_thread_id;

  error_no = copy_local(procp, child);
  if (error_no)
    {
      proc_dealloc_proc(child->proc_pid);
      kcall->thread_destroy(main_thread_id);
//TODO destroy process
      put_response (req->rdvno, error_no, -1, 0);
    }

  kcall->thread_start(main_thread_id);

  put_response (req->rdvno, EOK, child->proc_pid, 0);	/* 親プロセスに対して応答 */
}  

void if_kill(fs_request *req)
{
    struct proc *myprocp, *procp;
    W mypid, wpid, exst;
    W i;
    ER error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    /* req->caller が task 1 の場合は，返事のメッセージを送らない */

    mypid = req->packet.args.arg1;
    error_no = proc_get_procp(mypid, &myprocp);
    if (error_no) {
	put_response(req->rdvno, ESRCH, -1, 0);
	return;
    }
    myprocp->proc_exst = (-1);	/* 強制終了時のステータスは (-1) で良いか? */

    error_no = proc_get_procp(myprocp->proc_ppid, &procp);
    if (error_no) {
	put_response(req->rdvno, ESRCH, -1, 0);
	return;
    }
    wpid = procp->proc_wpid;
    if (procp->proc_status == PS_WAIT &&
	(wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
	/* 親プロセスが自分を WAIT していればメッセージ送信 */
	procp->proc_status = PS_RUN;
	exst = (myprocp->proc_exst << 8);
	put_response(req->rdvno, EOK, mypid, exst);

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
	put_response(req->rdvno, EOK, 0, 0);
    }
}

void
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
	put_response (req->rdvno, EOK, i, exst);
	
	/* 親プロセスの状態変更 */
	proc_get_procp(mypid, &procp);
	procp->proc_status = PS_RUN;

	/* 子プロセスのエントリーの開放 */
	proc_exit(i);
	
	return;
      }
    }
  }
  if (children > 0) {
    /* 対応する子プロセスはあったが，まだ終了していなかった */
    if (req->packet.param.par_waitpid.opts & WNOHANG) {
      /* 親に返事を送る必要がある */
      put_response (req->rdvno, EOK, 0, 0);
      return;
    }
    /* 親プロセスの状態を変更し，返事を送らずにシステムコールを終了 */
    proc_get_procp(mypid, &procp);
    procp->proc_status = PS_WAIT;
    procp->proc_wpid = pid;
    procp->proc_wait_rdvno = req->rdvno;
  }
  else {
    /* エラーを返す */
    put_response (req->rdvno, ECHILD, 0, 0);
  }
}  
