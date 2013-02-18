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

#include "thread.h"
#include "fs.h"

W psc_brk_f(RDVNO rdvno, struct posix_request *req)
{
    struct proc *myprocp;
    W err, mypid, i;
    T_REGION reg;
    VP start;
    UW size;

    mypid = req->procid;
    err = proc_get_procp(mypid, &myprocp);
    if (err)
	return err;

    err = vsts_reg(req->caller, HEAP_REGION, (VP) & reg);
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
	for (i = 0; i < PAGES(size); ++i) {
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
    W errno;

#ifdef EXEC_DEBUG
    printk("[PM] exec: start\n");
#endif

    /* パス名をユーザプロセスから POSIX サーバのメモリ空間へコピーする。
     */
    errno = vget_reg(req->caller, req->param.par_execve.name,
		     req->param.par_execve.pathlen + 1, pathname);
    if (errno) {
	/* パス名のコピーエラー */
	if (errno == E_PAR)
	    put_response(rdvno, EINVAL, -1, 0);
	else
	    put_response(rdvno, EFAULT, -1, 0);

	return (FALSE);
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
	return (FALSE);
    }

    /* プロセスに属するタスクを切り換える。
     * すなわち、古いタスクを終了し、新しいタスクを生成する。
     */


    /* exec システムコールは、成功すると元のプロセスは消えるので、
     * レスポンスを返さない
     */
    return (TRUE);
}

/* psc_exit_f - プロセスを終了させる
 */
W
psc_exit_f (RDVNO rdvno, struct posix_request *req)
{
  struct proc *myprocp, *procp;
  struct posix_request preq;
  W mypid, wpid, exst;
  W i;
  ER errno;
  ID tskid;

  mypid = req->procid;
  errno = proc_get_procp(mypid, &myprocp);
  if (errno) {
    put_response (rdvno, ESRCH, 0, 0);
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return errno;
  }

  myprocp->proc_exst = req->param.par_exit.evalue;

  errno = proc_get_procp(myprocp->proc_ppid, &procp);
  if (errno) {
    put_response (rdvno, ESRCH, 0, 0);
    /* メッセージの呼び出し元にエラーを返しても処理できないが，
       タスクは exd_tsk で終了する */
    return errno;
  }

  wpid = procp->proc_wpid;
  if (procp->proc_status == PS_WAIT &&
      (wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
    /* 親プロセスが自分を WAIT していればメッセージ送信 */
    procp->proc_status = PS_RUN;
    preq.operation = PSC_WAITPID;
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
  for(i = 1; i < MAX_PROCESS; ++i) {
    proc_get_procp(i, &procp);
    if (procp->proc_status == PS_DORMANT) continue;
    if (procp->proc_ppid != mypid) continue;
    procp->proc_ppid = 0; /* INIT プロセスの pid は 0 */
    
    /* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
  }

  /* POSIX の vmtree のみを開放．仮想メモリーは exd_tsk で開放 */
  destroy_proc_memory (myprocp, 0);

  /* region の開放, 実際には必要無いだろう */
  tskid = req->caller,
  vdel_reg(tskid, TEXT_REGION); /* text */
  vdel_reg(tskid, DATA_REGION); /* data+bss */
  vdel_reg(tskid, HEAP_REGION); /* heap */
#ifdef notdef
  vdel_reg(tskid, STACK_REGION); /* stack */
#endif

  put_response (rdvno, EOK, 0, 0);
  return (TRUE);
}  

/* psc_fork_f - 新しいプロセスを作成する
 */
W
psc_fork_f (RDVNO rdvno, struct posix_request *req)
{
  struct proc *procp;
  W	       errno;
  W	       childid;

  errno = proc_get_procp (req->procid, &procp);		/* 親プロセスの情報の取りだし */
  if (errno)
    {
      printk ("posix: invalid process id (%d)\n", req->procid);
      put_response (rdvno, errno, -1, 0);
      return (FALSE);
    }

#ifdef DEBUG
  printk ("psc_fork_f(): proc = 0x%x, proc->vm_tree = 0x%x\n", procp, procp->vm_tree);
#endif

  errno = proc_fork (procp, &childid, req->param.par_fork.main_task, req->param.par_fork.signal_task);
  if (errno)
    {
      put_response (rdvno, errno, -1, 0);
      return (FALSE);
    }

  put_response (rdvno, EOK, childid, 0);	/* 親プロセスに対して応答 */
  return (TRUE);
}  

W psc_kill_f(RDVNO rdvno, struct posix_request *req)
{
    struct proc *myprocp, *procp;
    struct posix_request preq;
    W mypid, wpid, exst;
    W i;
    ER errno;

    /* req->caller が task 1 の場合は，返事のメッセージを送らない */

    mypid = req->param.par_kill.pid;
    errno = proc_get_procp(mypid, &myprocp);
    if (errno) {
	return errno;
	if (req->caller != KERNEL_TASK) {
	    put_response(rdvno, ESRCH, 0, 0);
	}
    }
    myprocp->proc_exst = (-1);	/* 強制終了時のステータスは (-1) で良いか? */

    errno = proc_get_procp(myprocp->proc_ppid, &procp);
    if (errno) {
	return errno;
	if (req->caller != KERNEL_TASK) {
	    put_response(rdvno, ESRCH, 0, 0);
	}
    }
    wpid = procp->proc_wpid;
    if (procp->proc_status == PS_WAIT &&
	(wpid == -1 || wpid == mypid || -wpid == myprocp->proc_pgid)) {
	/* 親プロセスが自分を WAIT していればメッセージ送信 */
	procp->proc_status = PS_RUN;
	preq.operation = PSC_WAITPID;
	exst = (myprocp->proc_exst << 8);
	put_response(rdvno, EOK, mypid, exst);

	/* エントリーの開放 */
	proc_exit(mypid);
    } else {
	/* そうでなければ，ZOMBIE 状態に */
	myprocp->proc_status = PS_ZOMBIE;
    }

    /* 子プロセスの親を INIT に変更 */
    for (i = 1; i < MAX_PROCESS; ++i) {
	proc_get_procp(i, &procp);
	if (procp->proc_status == PS_DORMANT)
	    continue;
	if (procp->proc_ppid != mypid)
	    continue;
	procp->proc_ppid = 0;	/* INIT プロセスの pid は 0 */

	/* 子プロセスが ZOMBIE で INIT が wait していれば クリアする? */
    }

    /* POSIX の vmtree のみを開放．仮想メモリーは del_tsk で開放 */
    destroy_proc_memory(myprocp, 0);

    /* メインタスクの強制終了 */
    ter_tsk(myprocp->proc_maintask);
    del_tsk(myprocp->proc_maintask);

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
  for(i = 1; i < MAX_PROCESS; ++i) {
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
