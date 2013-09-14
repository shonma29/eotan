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
#include <kcall.h>
#include <setting.h>
#include <boot/init.h>
#include <mpu/config.h>
#include <mpu/memory.h>
#include "thread.h"
#include "fs.h"

W psc_brk_f(RDVNO rdvno, struct posix_request *req)
{
    struct proc *myprocp;
    W err, mypid, i;
    T_REGION reg;
    VP start;
    UW size;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    mypid = req->procid;
    err = proc_get_procp(mypid, &myprocp);
    if (err)
	return err;

    err = kcall->region_get_status(req->caller, HEAP_REGION, (VP) & reg);
#ifdef DEBUG
    dbg_printf("[PM] err = %d id %d, sa %x, min %x, max %x, ea %x\n",
	       req->caller,
	   err, reg.start_addr, reg.min_size, reg.max_size,
	   req->param.par_brk.end_adr);
#endif
    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

    err = EOK;
    start = reg.start_addr + reg.min_size;
    if (start > req->param.par_brk.end_adr) {
	/* region を縮小 */
	size = start - req->param.par_brk.end_adr;
	for (i = 1; i <= (size >> PAGE_SHIFT); ++i) {
	    err = shorten_vm(myprocp, (UW) start - (i << PAGE_SHIFT));
	    if (err)
		break;
	}
    } else if (start < req->param.par_brk.end_adr) {
	/* region を拡大 */
	size = req->param.par_brk.end_adr - start;
	for (i = 0; i < pages(size); ++i) {
	    err = grow_vm(myprocp, (UW) start + (i << PAGE_SHIFT),
			  VM_READ | VM_WRITE | VM_USER);
	    if (err)
		break;
	}
    }

    if (err) {
	put_response(rdvno, err, -1, 0);
	return (FALSE);
    }

#ifdef DEBUG
    /* for debug */
    vsts_reg(req->caller, HEAP_REGION, (VP) & reg);
    dbg_printf("[PM] after brk sa %x, min %x, max %x\n",
	   reg.start_addr, reg.min_size, reg.max_size);
#endif

    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

/* psc_exec_f - 指定されたプログラムファイルをメモリ中に読み込む
 */
W psc_exec_f(RDVNO rdvno, struct posix_request *req)
{
    B pathname[MAX_NAMELEN];
    W error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

#ifdef EXEC_DEBUG
    printk("[PM] exec: start\n");
#endif

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    error_no = kcall->region_get(req->caller, req->param.par_execve.name,
		     req->param.par_execve.pathlen + 1, pathname);
    if (error_no) {
	/* パス名のコピーエラー */
	if (error_no == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
    }
#ifdef EXEC_DEBUG
    printk("exec: pathname is %s\n", pathname);
#endif
    error_no = exec_program(req, req->procid, pathname);
    if (error_no) {
	if (proc_get_vmtree(req->procid) != NULL) {
	    /* 呼び出しを行ったプロセスがまだ生き残っていた場合 */
	    /*エラーメッセージを返す */
	    put_response(rdvno, error_no, -1, 0);
	} else {
	    /* 既にプロセスの仮想メモリが開放されている場合 */
	    /* exit が実行されることは無いので，ここで開放する */
	    proc_exit(req->procid);
	}
	return (FALSE);
    }

    /* プロセスに属するタスクを切り換える。
     * すなわち、古いタスクを終了し、新しいタスクを生成する。
     */

    put_response(rdvno, EOK, 0, 0);
    return (TRUE);
}

/* psc_exit_f - プロセスを終了させる
 */
W
psc_exit_f (RDVNO rdvno, struct posix_request *req)
{
  struct proc *myprocp, *procp;
  W mypid, wpid, exst;
  W i;
  ER error_no;
  ID tskid;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  mypid = req->procid;
  error_no = proc_get_procp(mypid, &myprocp);
  if (error_no) {
    put_response (rdvno, ESRCH, 0, 0);
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return error_no;
  }

  myprocp->proc_exst = req->param.par_exit.evalue;

  error_no = proc_get_procp(myprocp->proc_ppid, &procp);
  if (error_no) {
    put_response (rdvno, ESRCH, 0, 0);
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return error_no;
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
    
    /* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
  }

  /* POSIX の vmtree のみを開放．仮想メモリーは exd_tsk で開放 */
  destroy_proc_memory (myprocp, 0);

  /* region の開放, 実際には必要無いだろう */
  tskid = req->caller,
  kcall->region_destroy(tskid, TEXT_REGION); /* text */
  kcall->region_destroy(tskid, DATA_REGION); /* data+bss */
  kcall->region_destroy(tskid, HEAP_REGION); /* heap */
#ifdef notdef
  kcall->region_destroy(tskid, STACK_REGION); /* stack */
#endif

  kcall->thread_terminate(tskid);
  kcall->thread_destroy(tskid);

  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}  

/* psc_fork_f - 新しいプロセスを作成する
 */
W
psc_fork_f (RDVNO rdvno, struct posix_request *req)
{
  struct proc *procp;
  W	       error_no;
  ID main_thread_id;
  T_CTSK task_info = {
     TA_HLNG,
     NULL,
     req->param.par_fork.entry,
     pri_user_foreground,
     USER_STACK_SIZE,
     NULL
  };
  struct proc *child;
  kcall_t *kcall = (kcall_t*)KCALL_ADDR;

  error_no = proc_get_procp (req->procid, &procp);		/* 親プロセスの情報の取りだし */
  if (error_no)
    {
      printk ("posix: invalid process id (%d)\n", req->procid);
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

#ifdef DEBUG
  printk ("psc_fork_f(): proc = 0x%x, proc->vm_tree = 0x%x\n", procp, procp->vm_tree);
#endif

  error_no = proc_alloc_proc(&child);
  if (error_no)
    {
      printk ("posix: cannot allocate process\n");
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  task_info.exinf = child->proc_pid;
  main_thread_id = kcall->thread_create_auto(&task_info);
  if (main_thread_id < 0)
    {
      printk ("posix: acre_tsk error (%d)\n", main_thread_id);
      proc_dealloc_proc(child->proc_pid);
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  error_no = proc_fork (procp, child, main_thread_id, 0);
  if (error_no)
    {
      proc_dealloc_proc(child->proc_pid);
      kcall->thread_destroy(main_thread_id);
      put_response (rdvno, error_no, -1, 0);
      return (FALSE);
    }

  kcall->mpu_copy_stack(req->caller, (W)(req->param.par_fork.sp), main_thread_id);
  kcall->thread_start(main_thread_id, 0);

  put_response (rdvno, EOK, child->proc_pid, 0);	/* 親プロセスに対して応答 */
  return (TRUE);
}  

W psc_kill_f(RDVNO rdvno, struct posix_request *req)
{
    struct proc *myprocp, *procp;
    W mypid, wpid, exst;
    W i;
    ER error_no;
    kcall_t *kcall = (kcall_t*)KCALL_ADDR;

    /* req->caller が task 1 の場合は，返事のメッセージを送らない */

    mypid = req->param.par_kill.pid;
    error_no = proc_get_procp(mypid, &myprocp);
    if (error_no) {
	return error_no;
	if (req->caller != KERNEL_TASK) {
	    put_response(rdvno, ESRCH, 0, 0);
	}
    }
    myprocp->proc_exst = (-1);	/* 強制終了時のステータスは (-1) で良いか? */

    error_no = proc_get_procp(myprocp->proc_ppid, &procp);
    if (error_no) {
	return error_no;
	if (req->caller != KERNEL_TASK) {
	    put_response(rdvno, ESRCH, 0, 0);
	}
    }
    wpid = procp->proc_wpid;
    if (procp->proc_status == PS_WAIT &&
	(wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
	/* 親プロセスが自分を WAIT していればメッセージ送信 */
	procp->proc_status = PS_RUN;
	exst = (myprocp->proc_exst << 8);
	put_response(rdvno, EOK, mypid, exst);

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

	/* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
    }

    /* POSIX の vmtree のみを開放．仮想メモリーは del_tsk で開放 */
    destroy_proc_memory(myprocp, 0);

    /* メインタスクの強制終了 */
    kcall->thread_terminate(myprocp->proc_maintask);
    kcall->thread_destroy(myprocp->proc_maintask);

    if ((req->caller != KERNEL_TASK) &&
	(req->caller != myprocp->proc_maintask)) {
	put_response(rdvno, EOK, 0, 0);
    }
    return (TRUE);
}

W
psc_waitpid_f (RDVNO rdvno, struct posix_request *req)
{
  W i;
  W mypid, pid, children, exst;
  struct proc *procp;

  pid = req->param.par_waitpid.pid;
  mypid = req->procid;
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
	put_response (rdvno, EOK, i, exst);
	
	/* 親プロセスの状態変更 */
	proc_get_procp(mypid, &procp);
	procp->proc_status = PS_RUN;

	/* 子プロセスのエントリーの開放 */
	proc_exit(i);
	
	return(TRUE);
      }
    }
  }
  if (children > 0) {
    /* 対応する子プロセスはあったが，まだ終了していなかった */
    if (req->param.par_waitpid.opts & WNOHANG) {
      /* 親に返事を送る必要がある */
      put_response (rdvno, EOK, 0, 0);
      return (TRUE);
    }
    /* 親プロセスの状態を変更し，返事を送らずにシステムコールを終了 */
    proc_get_procp(mypid, &procp);
    procp->proc_status = PS_WAIT;
    procp->proc_wpid = pid;
    procp->proc_wait_rdvno = rdvno;
    return (TRUE);
  }
  else {
    /* エラーを返す */
    put_response (rdvno, ECHILD, 0, 0);
    return (FALSE);
  }
}  
