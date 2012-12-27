/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/common/syscall-if.c,v 1.23 2000/04/03 14:34:45 naniwa Exp $ */

/* syscall-if.c --- システムコールのインタフェース関数
 *
 * $Revision: 1.23 $
 * $Log: syscall-if.c,v $
 * Revision 1.23  2000/04/03 14:34:45  naniwa
 * to call timer handler in task
 *
 * Revision 1.22  2000/02/06 09:10:55  naniwa
 * minor fix
 *
 * Revision 1.21  1999/11/14 14:53:36  naniwa
 * add time management function
 *
 * Revision 1.20  1999/11/10 10:29:06  naniwa
 * to support execve, etc
 *
 * Revision 1.19  1999/07/30 08:18:29  naniwa
 * add vcpy_stk()
 *
 * Revision 1.18  1999/07/24 04:33:23  naniwa
 * add pmemstat
 *
 * Revision 1.17  1999/07/09 08:18:10  naniwa
 * modified sys_vsys_msc
 *
 * Revision 1.16  1999/04/18 17:48:31  monaka
 * Port-manager and libkernel.a is moved to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.15  1999/04/12 15:29:15  monaka
 * pointers to void are renamed to VP.
 *
 * Revision 1.14  1999/04/12 14:51:48  monaka
 * Added Redirection to virtual memory functions.
 *
 * Revision 1.13  1999/04/12 13:29:23  monaka
 * printf() is renamed to printk().
 *
 * Revision 1.12  1999/03/16 13:02:52  monaka
 * Modifies for source cleaning. Most of these are for avoid gcc's -Wall message.
 *
 * Revision 1.11  1998/02/25 12:40:31  night
 * vmap_reg () の引数の数がひとつ増えたことによる変更。
 *
 * Revision 1.10  1998/02/16 14:15:19  night
 * sys_vget_phs() 関数の追加。
 * この関数は、引数で指定した仮想アドレスにマッピングされている
 * 物理ページのアドレスを返す。
 *
 * Revision 1.9  1997/08/31 14:11:46  night
 * lowlib 関係の処理の追加
 *
 * Revision 1.8  1997/07/02 13:24:47  night
 * vput_reg の本体を登録
 *
 * Revision 1.7  1997/05/12 14:36:02  night
 * vget_reg システムコールが呼ばれた時に vget_reg() の実体を呼ぶように変
 * 更。
 * 今までは、E_NOSPT のエラーで返していた。
 *
 * Revision 1.6  1997/03/25 13:30:47  night
 * 関数のプロトタイプ宣言の追加および引数の不整合の修正
 *
 * Revision 1.5  1996/11/07  15:42:38  night
 * vset_cns の引数の指定方法が間違っていたので修正した。
 *
 * Revision 1.4  1996/11/07  12:41:28  night
 * 関数 sys_vsys_msc () の追加。
 * この関数はシステムコール vsys_msc に使用する。
 *
 * Revision 1.3  1996/11/06  12:41:00  night
 * sys_vset_cns() の追加。
 *
 * Revision 1.2  1996/09/11  18:27:12  night
 * sys_vget_csl() の追加。
 * この関数は、現在のカーソル位置を呼び出し元に返すシステムコール。
 *
 * Revision 1.1  1996/07/22  13:39:17  night
 * IBM PC 版 ITRON の最初の登録
 *
 * Revision 1.11  1995/12/13 16:02:03  night
 * vcre_reg, vmap_reg のシステムコールに対応する関数を呼び出すように
 * 修正した。
 *
 * Revision 1.10  1995/12/13 15:24:10  night
 * get_tid システムコールの実行時に出力していたデバッグ用の print 文を削
 * 除した。
 *
 * Revision 1.9  1995/10/01  12:57:18  night
 * def_int システムコール実行時のログメッセージを追加。
 *
 * Revision 1.8  1995/09/21  15:51:11  night
 * ソースファイルの先頭に Copyright notice 情報を追加。
 *
 * Revision 1.7  1995/09/19  18:03:35  night
 * 割り込みハンドラを登録するシステムコール sys_def_int() の変更。
 * 登録のための関数呼び出しを set_idt() から set_interrupt_entry() に変更
 * した。
 *
 * Revision 1.6  1995/03/18  13:59:40  night
 * def_int システムコールのための定義の追加。
 *
 * Revision 1.5  1995/02/26  13:56:43  night
 * 仮想メモリ管理関係のシステムコールインタフェース関数を追加。
 * 以下の関数を追加した。
 *
 * 	sys_vcre_reg, sys_vdel_reg, sys_vmap_reg, sys_vunm_reg,
 * 	sys_vdup_reg, sys_vprt_reg, sys_vshr_reg, sys_vput_reg,
 * 	sys_vget_reg, sys_vsts_reg
 *
 * Revision 1.4  1995/01/18  15:12:49  night
 * 以下の関数を追加。
 *
 *   sys_dis_int (), sys_ena_int (), sys_cre_mpl (), sys_del_mpl (),
 *   sys_get_blk (), sys_pget_blk (), sys_rel_blk (), sys_ref_mpl (),
 *   sys_tget_blk (), sys_get_ver ()
 *
 * Revision 1.3  1995/01/10  13:50:50  night
 * 以下のシステムコールインタフェース関数の追加。
 *
 * sys_cre_flg, sys_del_flg, sys_set_flg, sys_clr_flg,
 * sys_wai_flg, sys_pol_flg, sys_twai_flg, sys_ref_flg,
 * sys_cre_mbf, sys_del_mbf, sys_snd_mbf, sys_psnd_mbf,
 * sys_tsnd_mbf, sys_rcv_mbf, sys_prcv_mbf, sys_trcv_mbf
 *
 * Revision 1.2  1994/11/19  14:45:17  night
 * 以下のシステムコールインタフェース用関数を追加。
 *
 *   sys_sus_tsk,sys_rsm_tsk,sys_frsm_tsk,sys_slp_tsk,
 *   sys_wup_tsk,sys_can_wup,sys_cre_sem,sys_del_sem,
 *   sys_sig_sem,sys_wai_sem,sys_preq_sem,sys_twai_sem,
 *   sys_ref_sem
 *
 * これらの関数は割り込みスタックからシステムコールの引数を取り出し、
 * 実際に処理をする関数 ('sys_' を除いだ関数) を呼び出す。
 * 具体的には下のような形式の関数となる。
 *
 *  ER
 *  sys_foo (VOID *stackp)
 *  {
 *    struct arg
 *      {
 *        W	bar;
 *        W	baz;
 *      } *argp = (struct arg *)stackp;
 *
 *    return (foo (argp->bar, argp->baz));
 *  }
 *
 * Revision 1.1  1994/09/20  16:46:19  night
 * タスク管理関数の宣言。
 *
 *
 * 説明：
 * 本ファイルに含まれている関数は、ユーザレベルからシステムコールが
 * 発行された時にシステムコール関数として呼ばれる。
 * これらの関数は、引数のチェックを行い、実際の処理をする関数を呼び出す。
 *
 * システムコールインタフェース関数は、引数としてユーザスタックの先頭番地を
 * もらう。
 *
 */

#include "core.h"
#include "func.h"
#include "api.h"
#include "interrupt.h"
#include "../include/mpu/io.h"
#include "sync.h"
#include "boot.h"
#include "mpu/mpufunc.h"
#include "arch/archfunc.h"
#include "../include/itron/rendezvous.h"

#define SVC_IF(x,n)	if_ ## x
#define SVC_UNDEFINED		nodef

static ER	nodef (VP argp);

static ER	if_thread_create(void *argp);
static ER	if_thread_destroy(void *argp);
static ER	if_thread_start(void *argp);
static ER	if_thread_end(void *argp);
static ER	if_thread_end_and_destroy(void *argp);
static ER	if_thread_terminate(void *argp);
static ER	if_thread_change_priority(void *argp);
static ER	if_thread_release(void *argp);
static ER	if_thread_get_id(void *argp);
static ER	if_thread_suspend(void *argp);
static ER	if_thread_resume(void *argp);
static ER	if_flag_create_auto(void *argp);
static ER	if_flag_destroy(void *argp);
static ER	if_flag_set(void *argp);
static ER	if_flag_clear(void *argp);
static ER	if_flag_wait(void *argp);
static ER	if_queue_create(void *argp);
static ER	if_queue_create_auto(void *argp);
static ER	if_queue_destroy(void *argp);
static ER	if_message_send(void *argp);
static ER	if_message_send_nowait(void *argp);
static ER	if_message_receive(void *argp);

/* 時間管理用システムコール */
static ER	if_set_tim (void *argp);
static ER	if_get_tim (void *argp);
static ER	if_dly_tsk (void *argp);
static ER	if_def_alm (void *argp);

static ER	if_def_int (void *argp);

static ER	if_vsys_inf (void *argp);
static ER	if_dbg_puts (void *args);

/* 仮想メモリ管理用システムコール */
static ER	if_vcre_reg (void *argp);
static ER	if_vdel_reg (void *argp);
static ER	if_vmap_reg (void *argp);
static ER	if_vunm_reg (void *argp);
static ER	if_vdup_reg (void *argp);
static ER	if_vput_reg (void *argp);
static ER	if_vget_reg (void *argp);
static ER	if_vsts_reg (void *argp);
static ER	if_vget_phs (void *argp);

/* その他のシステムコール */
static ER	if_vsys_msc (void *argp);
static ER	if_vcpy_stk (void *argp);
static ER	if_vset_ctx (void *argp);
static ER	if_vuse_fpu (void *argp);

static ER if_port_create(void *argp);
static ER_ID if_port_create_auto(void *argp);
static ER if_port_destroy(void *argp);
static ER_UINT if_port_call(void *argp);
static ER_UINT if_port_accept(void *argp);
static ER if_port_reply(void *argp);


/* システムコールテーブル
 */  
static ER (*syscall_table[])(VP argp) =
{
  SVC_UNDEFINED,		/*    0 */

  /* タスク管理システムコール */
  SVC_IF (thread_create, 2),	/*    1 */
  SVC_IF (thread_destroy, 1),	/*    2 */
  SVC_IF (thread_start, 2),	/*    3 */
  SVC_IF (thread_end, 0),     /*    4 */
  SVC_IF (thread_end_and_destroy, 0),     /*    5 */
  SVC_IF (thread_terminate, 1),     /*    6 */
  SVC_IF (thread_change_priority, 2),	/*    7 */
  SVC_IF (thread_release, 1),	/*    8 */
  SVC_IF (thread_get_id, 1),	/*    9 */

  /* タスク附属同期機能 */
  SVC_IF (thread_suspend, 1),    	/*   10 */
  SVC_IF (thread_resume, 1),	/*   11 */

  /* 同期・通信機構 */
  /* セマフォ */

  /* イベントフラグ */
  SVC_IF (flag_create_auto, 1),	/*   12 */
  SVC_IF (flag_destroy, 1),	/*   13 */
  SVC_IF (flag_set, 2),	/*   14 */
  SVC_IF (flag_clear, 2),     /*   15 */
  SVC_IF (flag_wait, 4),	/*   16 */

  /* メッセージバッファ */
  SVC_IF (queue_create, 2),	/*   17	*/
  SVC_IF (queue_create_auto, 1),	/*   18	*/
  SVC_IF (queue_destroy, 1),	/*   19 */
  SVC_IF (message_send, 3),	/*   20 */
  SVC_IF (message_send_nowait, 3),	/*   21 */
  SVC_IF (message_receive, 3),	/*   22 */

  /* 割りこみ管理 */

  /* メモリ管理 */

  /* システム管理 */

  /* 時間管理機能 */
  SVC_IF (set_tim, 1),	/*   23 */
  SVC_IF (get_tim, 1),	/*   24 */
  SVC_IF (dly_tsk, 1),	/*   25 */
  SVC_IF (def_alm, 2),	/*   26 */

  SVC_IF (def_int, 2),	/*   27 */

  SVC_IF (vsys_inf, 3),	/*   28 */
  SVC_IF (dbg_puts, 1),	/*   29 */

  /* 仮想メモリ管理システムコール */	
  SVC_IF (vcre_reg, 7),	/*   30 */
  SVC_IF (vdel_reg, 2),	/*   31 */
  SVC_IF (vmap_reg, 3),	/*   32 */
  SVC_IF (vunm_reg, 3),	/*   33 */
  SVC_IF (vdup_reg, 3),	/*   34 */
  SVC_IF (vput_reg, 4),	/*   35 */
  SVC_IF (vget_reg, 4),	/*   36 */
  SVC_IF (vsts_reg, 3),	/*   37 */
  SVC_IF (vget_phs, 3),	/*   38 */

  /* その他のシステムコール */
  SVC_IF (vsys_msc, 2),	/*   39 */
  SVC_IF (vcpy_stk, 4),	/*   40 */
  SVC_IF (vset_ctx, 4),	/*   41 */
  SVC_IF (vuse_fpu, 1),	/*   42 */

  SVC_IF (port_create, 2),	/*   43 */
  SVC_IF (port_create_auto, 1),	/*   44 */
  SVC_IF (port_destroy, 1),	/*   45 */
  SVC_IF (port_call, 4),	/*   46 */
  SVC_IF (port_accept, 4),	/*   47 */
  SVC_IF (port_reply, 3),	/*   48 */
};

#define NSYSCALL (sizeof (syscall_table) / sizeof (syscall_table[0]))


/**************************************************************************
 *	syscall --- システムコールの処理部分
 *
 *	呼び出し関数: interrupt (machine/interrupt.s)
 *
 *	システムコールの要求をユーザスタックより取り出し、適当に処理を振り
 *	分ける。
 *
 *	引数:
 *		sysno		システムコール番号
 *		arg_addr	ユーザスタックの top の番地
 */
W
syscall (UW sysno, W *arg_addr)
{
  W	errno;

/* システムコール番号のチェック 
 */
  if ((sysno <= 0) || (sysno > NSYSCALL))
    {
      return (E_OBJ);
    }

  errno = (syscall_table[sysno])(arg_addr);

  /* 
   * システムコール処理用のデバッグ文。
   */
#ifdef notdef  
  printk ("%s, %d, syscall: called. (errno = %d)\n", 
	  __FILE__, __LINE__, errno);		/* R.Naitoh */
#endif /* notdef */

  return (errno);
}

/* --------------------------------------------------------------------- */
static ER
nodef (VP argp)
{
  return (E_NOSPT);
}


/* ----------------------------------------------------------------------- *
 * タスク関係システムコール                                                *
 * ----------------------------------------------------------------------- */

/* if_thread_create --- タスクの生成
 *
 * 引数: tskid 		生成するタスクのID
 *	 pk_ctsk	生成するタスクの属性情報
 *			tskatr		タスク属性
 *			startaddr	タスク起動アドレス
 *			itskpri		タスク起動時優先度
 *			stksz		スタックサイズ
 *			addrmap		アドレスマップ
 *	
 */
static ER if_thread_create(VP argp)
{
    struct {
	ID tskid;
	T_CTSK *tskpkt;
    } *args = argp;

    return (thread_create(args->tskid, args->tskpkt));
}

/* if_thread_destroy --- 指定したタスクを削除
 *
 * 引数：tskid	削除するタスクの ID
 *
 */
static ER if_thread_destroy(VP argp)
{
    struct {
	ID tskid;
    } *args = argp;

    return (thread_destroy(args->tskid));
}


/* if_thread_start --- タスクの状態を取り出す
 *
 * 引数：tskid	状態を取り出すタスの ID
 *	 stacd  状態を取り出す領域
 *
 */
static ER if_thread_start(VP argp)
{
    struct {
	ID tskid;
	INT stacd;
    } *args = argp;

    return (thread_start(args->tskid, args->stacd));
}

/* if_thread_end --- 自タスクを終了する
 *
 */
static ER if_thread_end(VP argp)
{
    thread_end();
    return (E_OK);		/* 本当は、返り値はないが... */
}

/* if_exd_tsk --- 自タスクを終了して、資源を解放する。
 *
 */
static ER if_thread_end_and_destroy(VP argp)
{
    thread_end_and_destroy();
    return (E_OK);		/* 本当は、返り値はないが... */
}

/* if_thread_terminate --- 指定したタスクを終了する
 *
 * 引数：tskid	終了するタスクの ID
 *
 */
static ER if_thread_terminate(VP argp)
{
    struct {
	ID tskid;
    } *args = argp;

    return (thread_terminate(args->tskid));
}


static ER if_thread_change_priority(VP argp)
{
    struct {
	ID tskid;
	PRI tskpri;
    } *args = argp;

    return (thread_change_priority(args->tskid, args->tskpri));
}

static ER if_thread_release(VP argp)
{
    struct {
	ID tskid;
    } *args = argp;

    return (thread_release(args->tskid));
}

static ER if_thread_get_id(VP argp)
{
    struct {
	ID *p_tskid;
    } *args = argp;
    ID rid;
    ER err;

    err = thread_get_id(&rid);
    if (err == E_OK)
	err = vput_reg(run_task->tskid, args->p_tskid, sizeof(ID), &rid);
    return (err);
}

static ER if_thread_suspend(VP argp)
{
    struct {
	ID taskid;
    } *args = argp;

    return (thread_suspend(args->taskid));
}


static ER if_thread_resume(VP argp)
{
    struct {
	ID taskid;
    } *args = argp;

    return (thread_resume(args->taskid));
}


/* ----------------------------------------------------------------------- *
 * タスク間通信システムコール                                              *
 * ----------------------------------------------------------------------- */
static ER if_flag_create_auto(VP argp)
{
    struct {
	T_CFLG *pk_cflg;
    } *args = argp;

    return (flag_create_auto(args->pk_cflg));
}

static ER if_flag_destroy(VP argp)
{
    struct {
	ID flgid;
    } *args = argp;

    return (flag_destroy(args->flgid));
}

static ER if_flag_set(VP argp)
{
    struct {
	ID flgid;
	UINT setptn;
    } *args = argp;

    return (flag_set(args->flgid, args->setptn));
}

static ER if_flag_clear(VP argp)
{
    struct {
	ID flgid;
	UINT clrptn;
    } *args = argp;

    return (flag_clear(args->flgid, args->clrptn));
}

static ER if_flag_wait(VP argp)
{
    struct {
	UINT *p_flgptn;
	ID flgid;
	UINT waiptn;
	UINT wfmode;
    } *args = argp;

    return (flag_wait
	    (args->p_flgptn, args->flgid, args->waiptn, args->wfmode));
}

static ER if_queue_create(VP argp)
{
    struct {
	ID id;
	T_CMBF *pk_cmbf;
    } *args = argp;

    return (queue_create(args->id, args->pk_cmbf));
}

static ER if_queue_create_auto(VP argp)
{
    struct {
	T_CMBF *pk_cmbf;
    } *args = argp;

    return (queue_create_auto(args->pk_cmbf));
}

static ER if_queue_destroy(VP argp)
{
    struct {
	ID id;
    } *args = argp;

    return (queue_destroy(args->id));
}

static ER if_message_send(VP argp)
{
    struct {
	ID id;
	INT size;
	VP msg;
    } *args = argp;

    return (message_send(args->id, args->size, args->msg));
}

static ER if_message_send_nowait(VP argp)
{
    struct {
	ID id;
	INT size;
	VP msg;
    } *args = argp;

    return (message_send_nowait(args->id, args->size, args->msg));
}

static ER if_message_receive(VP argp)
{
    struct {
	VP msg;
	INT *size;
	ID id;
    } *args = argp;

    return (message_receive(args->msg, args->size, args->id));
}

/*
 * 割り込みハンドラを定義する。
 *
 * この関数では、今のところ CPU レベル 0/トラップ 形式の割り込みとして
 * ハンドラを設定している。
 *
 */
static ER if_def_int(VP argp)
{
    struct a {
	UINT dintno;
	T_DINT *pk_dint;
    } *args = (struct a *) argp;

    if ((args->dintno < 0) || (args->dintno > 128)) {
	return (E_PAR);
    }
#ifdef notdef
    printk("if_def_int: dintno = %d(0x%x), inthdr = 0x%x, intatr = %d\n",
	   args->dintno,
	   args->dintno, args->pk_dint->inthdr, args->pk_dint->intatr);
#endif

    set_interrupt_entry(args->dintno,
			args->pk_dint->inthdr, args->pk_dint->intatr);
    return (E_OK);
}

/*
 * 仮想メモリ管理の関数群 
 */

/*
 *
 */
static ER if_vcre_reg(VP argp)
{
    struct {
	ID id;
	ID rid;
	VP start;
	W min;
	W max;
	UW perm;
	FP handle;
    } *args = argp;

    return vcre_reg(args->id, args->rid, args->start, args->min,
		    args->max, args->perm, args->handle);
}

/*
 *
 */
static ER if_vdel_reg(VP argp)
{
    struct {
	ID id;
	ID rid;
    } *args = argp;

    return vdel_reg(args->id, args->rid);
}

/*
 *
 */
static ER if_vmap_reg(VP argp)
{
    struct {
	ID id;
	VP start;
	W size;
	W accmode;
    } *args = argp;

    return vmap_reg(args->id, args->start, args->size, args->accmode);
}

/*
 *
 */
static ER if_vunm_reg(VP argp)
{
    struct {
	ID id;
	VP addr;
	W size;
    } *args = argp;

    return vunm_reg(args->id, args->addr, args->size);
}

/*
 *
 */
static ER if_vdup_reg(VP argp)
{
    struct {
	ID src;
	ID dst;
	ID rid;
    } *args = argp;

    return vdup_reg(args->src, args->dst, args->rid);
}

/*
 *
 */
static ER if_vput_reg(VP argp)
{
    struct {
	ID id;
	VP start;
	W size;
	VP buf;
    } *args = argp;

    return vput_reg(args->id, args->start, args->size, args->buf);
}

/*
 *
 */
static ER if_vget_reg(VP argp)
{
    struct {
	ID id;
	VP start;
	W size;
	VP buf;
    } *args = argp;


    return vget_reg(args->id, args->start, args->size, args->buf);
}

/*
 *
 */
static ER if_vsts_reg(VP argp)
{
    struct {
	ID id;
	ID rid;
	VP stat;
    } *args = argp;

    return vsts_reg(args->id, args->rid, args->stat);
}



/*
 *
 */
static ER if_vget_phs(VP argp)
{
    struct {
	ID id;
	VP addr;
	UW *paddr;
    } *args = argp;
    UW physical_addr;

    physical_addr = vtor(args->id, (UW) args->addr);
    if (physical_addr == NULL) {
	return (E_OBJ);
    }
    *(args->paddr) = physical_addr;
    return (E_OK);
}


/*
 * 時間管理の関数群 
 */

static ER if_set_tim(VP argp)
{
    struct {
	SYSTIME *pk_tim;
    } *args = argp;

    return (set_tim(args->pk_tim));
}

static ER if_get_tim(VP argp)
{
    struct {
	SYSTIME *pk_tim;
    } *args = argp;

    return (get_tim(args->pk_tim));
}

static ER if_dly_tsk(VP argp)
{
    struct {
	DLYTIME dlytim;
    } *args = argp;

    return (dly_tsk(args->dlytim));
}

static ER if_def_alm(VP argp)
{
    struct {
	HNO almo;
	T_DALM *pk_dalm;
    } *args = argp;

    return (def_alm(args->almo, args->pk_dalm));
}

/*
 * その他の関数群 
 */
/*
 *
 */
static ER
if_vsys_inf (VP argp)
{
  struct 
    {
      W func;
      W sub;
      VP buf;
    } *args = argp;
  W		  *rootfs;
  struct boot_header	*info;
  

  switch (args->func)
    {
      
    case ROOTFS_INFO:
      rootfs = (W *)args->buf;
      info = (struct boot_header *)(MODULE_TABLE | 0x80000000);
      *rootfs = info->machine.rootfs;
      printk ("vsys_inf: rootfs_info: rootfs = 0x%x\n", info->machine.rootfs);	/* */
      return (E_OK);

    default:
      return (E_PAR);
    }

}

static ER
if_dbg_puts (VP args)
{
  struct a
    {
      B *msg;
    } *arg = (struct a *)args;
  
  printk ("%s", arg->msg);
  return (E_OK);
}

/* if_vsys_msc - miscellaneous function
 *
 */
static ER if_vsys_msc(VP argp)
{
    struct {
	W cmd;
	VP arg;
    } *args = argp;

#ifdef notdef
    printk("vsys_msc: cmd = %d\n", args->cmd);
#endif

    switch (args->cmd) {
    default:
	return (E_ID);

    case 1:
	/* fall down */
	falldown(args->arg);
	enter_critical();
	for (;;);
	/* NOT REACHED */

    case 2:
	/* reset */
        system_reset();
	/* NOT REACHED */
	break;

    case 3:
	/* load lowlib */
	return (load_lowlib(args->arg));
	break;

    case 4:
	/* unload lowlib */
	return E_NOSPT;
	break;

    case 5:
	/* status lowlib */
	return (stat_lowlib(args->arg));
	break;

    case 6:
	/* print_thread_list */
	print_thread_list();
	break;

    case 7:
	/* pmemstat */
	pmemstat();
	break;
    }

    return (E_OK);
}

/* if_vcpy_stk - copy task stack 
 *
 */
static ER if_vcpy_stk(VP argp)
{
    struct {
	ID src;
	W esp; W ebp;
        W ebx; W ecx; W edx; W esi; W edi;
	ID dst;
    } *args = argp;

    return vcpy_stk(args->src, args->esp, args->ebp,
		    args->ebx, args->ecx, args->edx,
		    args->esi, args->edi, args->dst);
}

/* if_vset_cxt - set task context
 *
 */
static ER if_vset_ctx(VP argp)
{
    struct {
	ID tid;
	W eip;
	B *stackp;
	W stsize;
    } *args = argp;

    return vset_ctx(args->tid, args->eip, args->stackp, args->stsize);
}

/* if_vuse_fpu - use FPU
 *
 */
static ER if_vuse_fpu(VP argp)
{
    struct {
	ID tid;
    } *args = argp;

    return vuse_fpu(args->tid);
}

static ER if_port_create(VP argp)
{
    struct {
	ID porid;
	T_CPOR *pk_cpor;
    } *args = argp;

    return port_create(args->porid, args->pk_cpor);
}

static ER_ID if_port_create_auto(VP argp)
{
    struct {
	T_CPOR *pk_cpor;
    } *args = argp;

    return port_create_auto(args->pk_cpor);
}

static ER if_port_destroy(VP argp)
{
    struct {
	ID porid;
    } *args = argp;

    return port_destroy(args->porid);
}

static ER_UINT if_port_call(VP argp)
{
    struct {
	ID porid;
	RDVPTN calptn;
	VP msg;
	UINT cmsgsz;
    } *args = argp;

    return port_call(args->porid, args->calptn, args->msg, args->cmsgsz);
}

static ER_UINT if_port_accept(VP argp)
{
    struct {
	ID porid;
	RDVPTN calptn;
	RDVNO *p_rdvno;
	VP msg;
    } *args = argp;

    return port_accept(args->porid, args->p_rdvno, args->msg);
}

static ER if_port_reply(VP argp)
{
    struct {
	RDVNO rdvno;
	VP msg;
	UINT rmsgsz;
    } *args = argp;

    return port_reply(args->rdvno, args->msg, args->rmsgsz);
}
