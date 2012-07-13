/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/common/syscall-if.c,v 1.23 2000/04/03 14:34:45 naniwa Exp $ */
static char rcsid[] =
    "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/common/syscall-if.c,v 1.23 2000/04/03 14:34:45 naniwa Exp $";

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
#include "../../include/mpu/io.h"
#include "boot.h"

#define DEF_SYSCALL(x,n)	sys_ ## x
#define DEF_NOSYSCALL		nodef

static ER	nodef (VP argp);

static ER	sys_cre_tsk (void *argp);
static ER	sys_del_tsk (void *argp);
static ER	sys_sta_tsk (void *argp);
static ER	sys_ext_tsk (void *argp);
static ER	sys_exd_tsk (void *argp);
static ER	sys_ter_tsk (void *argp);
static ER	sys_dis_dsp (void *argp);
static ER	sys_ena_dsp (void *argp);
static ER	sys_chg_pri (void *argp);
static ER	sys_rot_rdq (void *argp);
static ER	sys_rel_wai (void *argp);
static ER	sys_get_tid (void *argp);
static ER	sys_sus_tsk (void *argp);
static ER	sys_rsm_tsk (void *argp);
static ER	sys_slp_tsk (void *argp);
static ER	sys_wup_tsk (void *argp);
static ER	sys_can_wup (void *argp);
static ER	sys_cre_flg (void *argp);
static ER	sys_acre_flg (void *argp);
static ER	sys_del_flg (void *argp);
static ER	sys_set_flg (void *argp);
static ER	sys_clr_flg (void *argp);
static ER	sys_wai_flg (void *argp);
static ER	sys_cre_mbf (void *argp);
static ER	sys_acre_mbf (void *argp);
static ER	sys_del_mbf (void *argp);
static ER	sys_snd_mbf (void *argp);
static ER	sys_psnd_mbf (void *argp);
static ER	sys_rcv_mbf (void *argp);

/* 時間管理用システムコール */
static ER	sys_set_tim (void *argp);
static ER	sys_get_tim (void *argp);
static ER	sys_dly_tsk (void *argp);
static ER	sys_def_alm (void *argp);

static ER	sys_def_int (void *argp);

static ER	sys_vsys_inf (void *argp);
static ER	sys_dbg_puts (void *args);

/* 仮想メモリ管理用システムコール */
static ER	sys_vcre_reg (void *argp);
static ER	sys_vdel_reg (void *argp);
static ER	sys_vmap_reg (void *argp);
static ER	sys_vunm_reg (void *argp);
static ER	sys_vdup_reg (void *argp);
static ER	sys_vprt_reg (void *argp);
static ER	sys_vshr_reg (void *argp);
static ER	sys_vput_reg (void *argp);
static ER	sys_vget_reg (void *argp);
static ER	sys_vsts_reg (void *argp);
static ER	sys_vget_phs (void *argp);

/* その他のシステムコール */
static ER	sys_vsys_msc (void *argp);
static ER	sys_vcpy_stk (void *argp);
static ER	sys_vset_ctx (void *argp);
static ER	sys_vuse_fpu (void *argp);

/* システムコールテーブル
 */  
static ER (*syscall_table[])(VP argp) =
{
  DEF_NOSYSCALL,		/*    0 */

  /* タスク管理システムコール */
  DEF_SYSCALL (cre_tsk, 2),	/*    1 */
  DEF_SYSCALL (del_tsk, 1),	/*    2 */
  DEF_SYSCALL (sta_tsk, 2),	/*    3 */
  DEF_SYSCALL (ext_tsk, 0),     /*    4 */
  DEF_SYSCALL (exd_tsk, 0),     /*    5 */
  DEF_SYSCALL (ter_tsk, 1),     /*    6 */
  DEF_SYSCALL (dis_dsp, 0),	/*    7 */
  DEF_SYSCALL (ena_dsp, 0),	/*    8 */
  DEF_SYSCALL (chg_pri, 2),	/*    9 */
  DEF_SYSCALL (rot_rdq, 1),	/*   10 */
  DEF_SYSCALL (rel_wai, 1),	/*   11 */
  DEF_SYSCALL (get_tid, 1),	/*   12 */

  /* タスク附属同期機能 */
  DEF_SYSCALL (sus_tsk, 1),    	/*   13 */
  DEF_SYSCALL (rsm_tsk, 1),	/*   14 */
  DEF_SYSCALL (slp_tsk, 0),	/*   15 */
  DEF_SYSCALL (wup_tsk, 1),	/*   16 */
  DEF_SYSCALL (can_wup, 2),	/*   17 */
  
  /* 同期・通信機構 */
  /* セマフォ */

  /* イベントフラグ */
  DEF_SYSCALL (cre_flg, 2),	/*   18 */
  DEF_SYSCALL (del_flg, 1),	/*   19 */
  DEF_SYSCALL (set_flg, 2),	/*   20 */
  DEF_SYSCALL (clr_flg, 2),     /*   21 */
  DEF_SYSCALL (wai_flg, 4),	/*   22 */

  /* メッセージバッファ */
  DEF_SYSCALL (cre_mbf, 2),	/*   23	*/
  DEF_SYSCALL (del_mbf, 1),	/*   24 */
  DEF_SYSCALL (snd_mbf, 3),	/*   25 */
  DEF_SYSCALL (psnd_mbf, 3),	/*   26 */
  DEF_SYSCALL (rcv_mbf, 3),	/*   27 */

  /* 割りこみ管理 */

  /* メモリ管理 */

  /* システム管理 */

  /* 時間管理機能 */
  DEF_SYSCALL (set_tim, 1),	/*   30 set_tim */
  DEF_SYSCALL (get_tim, 1),	/*   31 get_tim */
  DEF_SYSCALL (dly_tsk, 1),	/*   32 dly_tsk */
  DEF_SYSCALL (def_alm, 2),	/*   33 def_alm */

  DEF_SYSCALL (def_int, 2),	/*   34 */

  DEF_SYSCALL (vsys_inf, 3),	/*   35 */
  DEF_SYSCALL (dbg_puts, 1),	/*   36 */

  /* 仮想メモリ管理システムコール */	
  DEF_SYSCALL (vcre_reg, 7),	/*   37 */
  DEF_SYSCALL (vdel_reg, 2),	/*   38 */
  DEF_SYSCALL (vmap_reg, 3),	/*   39 */
  DEF_SYSCALL (vunm_reg, 3),	/*   40 */
  DEF_SYSCALL (vdup_reg, 3),	/*   41 */
  DEF_SYSCALL (vprt_reg, 3),	/*   42 */
  DEF_SYSCALL (vshr_reg, 3),	/*   43 */
  DEF_SYSCALL (vput_reg, 4),	/*   44 */
  DEF_SYSCALL (vget_reg, 4),	/*   45 */
  DEF_SYSCALL (vsts_reg, 3),	/*   46 */
  DEF_SYSCALL (vget_phs, 3),	/*   47 */

  /* その他のシステムコール */
  DEF_SYSCALL (vsys_msc, 2),	/*   48 */
  DEF_SYSCALL (vcpy_stk, 4),	/*   49 */
  DEF_SYSCALL (vset_ctx, 4),	/*   50 */
  DEF_SYSCALL (vuse_fpu, 1),	/*   51 */

  DEF_SYSCALL (acre_flg, 1),	/*   52 */
  DEF_SYSCALL (acre_mbf, 1),	/*   53	*/
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

/* sys_cre_tsk --- タスクの生成
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
static ER sys_cre_tsk(VP argp)
{
    struct {
	ID tskid;
	T_CTSK *tskpkt;
    } *args = argp;

    return (cre_tsk(args->tskid, args->tskpkt));
}

/* sys_del_tsk --- 指定したタスクを削除
 *
 * 引数：tskid	削除するタスクの ID
 *
 */
static ER sys_del_tsk(VP argp)
{
    struct {
	ID tskid;
    } *args = argp;

    return (del_tsk(args->tskid));
}


/* sys_sta_tsk --- タスクの状態を取り出す
 *
 * 引数：tskid	状態を取り出すタスの ID
 *	 stacd  状態を取り出す領域
 *
 */
static ER sys_sta_tsk(VP argp)
{
    struct {
	ID tskid;
	INT stacd;
    } *args = argp;

    return (sta_tsk(args->tskid, args->stacd));
}

/* sys_ext_tsk --- 自タスクを終了する
 *
 */
static ER sys_ext_tsk(VP argp)
{
    ext_tsk();
    return (E_OK);		/* 本当は、返り値はないが... */
}

/* sys_exd_tsk --- 自タスクを終了して、資源を解放する。
 *
 */
static ER sys_exd_tsk(VP argp)
{
    exd_tsk();
    return (E_OK);		/* 本当は、返り値はないが... */
}

/* sys_ter_tsk --- 指定したタスクを終了する
 *
 * 引数：tskid	終了するタスクの ID
 *
 */
static ER sys_ter_tsk(VP argp)
{
    struct {
	ID tskid;
    } *args = argp;

    return (ter_tsk(args->tskid));
}


static ER sys_dis_dsp(VP argp)
{
    return (dis_dsp());
}

static ER sys_ena_dsp(VP argp)
{
    return (ena_dsp());
}

static ER sys_chg_pri(VP argp)
{
    struct {
	ID tskid;
	PRI tskpri;
    } *args = argp;

    return (chg_pri(args->tskid, args->tskpri));
}

static ER sys_rot_rdq(VP argp)
{
    struct {
	PRI tskpri;
    } *args = argp;

    return (rot_rdq(args->tskpri));
}

static ER sys_rel_wai(VP argp)
{
    struct {
	ID tskid;
    } *args = argp;

    return (rel_wai(args->tskid));
}

static ER sys_get_tid(VP argp)
{
    struct {
	ID *p_tskid;
    } *args = argp;
    ID rid;
    ER err;

    err = get_tid(&rid);
    if (err == E_OK)
	err = vput_reg(run_task->tskid, args->p_tskid, sizeof(ID), &rid);
    return (err);
}

static ER sys_sus_tsk(VP argp)
{
    struct {
	ID taskid;
    } *args = argp;

    return (sus_tsk(args->taskid));
}


static ER sys_rsm_tsk(VP argp)
{
    struct {
	ID taskid;
    } *args = argp;

    return (rsm_tsk(args->taskid));
}


static ER sys_slp_tsk(VP argp)
{
    return (slp_tsk());
}


#ifdef notdef
static ER sys_tslp_tsk(VP argp)
{
}
#endif				/* notdef */


static ER sys_wup_tsk(VP argp)
{
    struct {
	ID taskid;
    } *args = argp;

    return (wup_tsk(args->taskid));
}


static ER sys_can_wup(VP argp)
{
    struct {
	INT *p_wupcnt;
	ID taskid;
    } *args = argp;

    return (can_wup(args->p_wupcnt, args->taskid));
}


/* ----------------------------------------------------------------------- *
 * タスク間通信システムコール                                              *
 * ----------------------------------------------------------------------- */
static ER sys_cre_flg(VP argp)
{
    struct {
	ID flgid;
	T_CFLG *pk_cflg;
    } *args = argp;

    return (cre_flg(args->flgid, args->pk_cflg));
}

static ER sys_acre_flg(VP argp)
{
    struct {
	T_CFLG *pk_cflg;
    } *args = argp;

    return (acre_flg(args->pk_cflg));
}

static ER sys_del_flg(VP argp)
{
    struct {
	ID flgid;
    } *args = argp;

    return (del_flg(args->flgid));
}

static ER sys_set_flg(VP argp)
{
    struct {
	ID flgid;
	UINT setptn;
    } *args = argp;

    return (set_flg(args->flgid, args->setptn));
}

static ER sys_clr_flg(VP argp)
{
    struct {
	ID flgid;
	UINT clrptn;
    } *args = argp;

    return (clr_flg(args->flgid, args->clrptn));
}

static ER sys_wai_flg(VP argp)
{
    struct {
	UINT *p_flgptn;
	ID flgid;
	UINT waiptn;
	UINT wfmode;
    } *args = argp;

    return (wai_flg
	    (args->p_flgptn, args->flgid, args->waiptn, args->wfmode));
}

static ER sys_cre_mbf(VP argp)
{
    struct {
	ID id;
	T_CMBF *pk_cmbf;
    } *args = argp;

    return (cre_mbf(args->id, args->pk_cmbf));
}

static ER sys_acre_mbf(VP argp)
{
    struct {
	T_CMBF *pk_cmbf;
    } *args = argp;

    return (acre_mbf(args->pk_cmbf));
}

static ER sys_del_mbf(VP argp)
{
    struct {
	ID id;
    } *args = argp;

    return (del_mbf(args->id));
}

static ER sys_snd_mbf(VP argp)
{
    struct {
	ID id;
	INT size;
	VP msg;
    } *args = argp;

    return (snd_mbf(args->id, args->size, args->msg));
}

static ER sys_psnd_mbf(VP argp)
{
    struct {
	ID id;
	INT size;
	VP msg;
    } *args = argp;

    return (psnd_mbf(args->id, args->size, args->msg));
}

static ER sys_rcv_mbf(VP argp)
{
    struct {
	VP msg;
	INT *size;
	ID id;
    } *args = argp;

    return (rcv_mbf(args->msg, args->size, args->id));
}

/*
 * 割り込みハンドラを定義する。
 *
 * この関数では、今のところ CPU レベル 0/トラップ 形式の割り込みとして
 * ハンドラを設定している。
 *
 */
static ER sys_def_int(VP argp)
{
    struct a {
	UINT dintno;
	T_DINT *pk_dint;
    } *args = (struct a *) argp;

    if ((args->dintno < 0) || (args->dintno > 128)) {
	return (E_PAR);
    }
#ifdef notdef
    printk("sys_def_int: dintno = %d(0x%x), inthdr = 0x%x, intatr = %d\n",
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
static ER sys_vcre_reg(VP argp)
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
static ER sys_vdel_reg(VP argp)
{
    struct {
	ID id;
#ifdef notdef
	VP start;
#else
	ID rid;
#endif
    } *args = argp;
#ifdef notdef
    return vdel_reg(args->id, args->start);
#else
    return vdel_reg(args->id, args->rid);
#endif
}

/*
 *
 */
static ER sys_vmap_reg(VP argp)
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
static ER sys_vunm_reg(VP argp)
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
static ER sys_vdup_reg(VP argp)
{
    struct {
	ID src;
	ID dst;
#ifdef notdef
	VP start;
#else
	ID rid;
#endif
    } *args = argp;

#ifdef notdef
    return vdup_reg(args->src, args->dst, args->start);
#else
    return vdup_reg(args->src, args->dst, args->rid);
#endif
}

/*
 *
 */
static ER sys_vprt_reg(VP argp)
{
    struct {
	ID id;
	VP start;
	UW prot;
    } *args = argp;

    return vprt_reg(args->id, args->start, args->prot);
}

/*
 *
 */
static ER sys_vshr_reg(VP argp)
{
    struct {
	ID src;
	ID dst;
	VP start;
    } *args = argp;

    return vshr_reg(args->src, args->dst, args->start);
}

/*
 *
 */
static ER sys_vput_reg(VP argp)
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
static ER sys_vget_reg(VP argp)
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
static ER sys_vsts_reg(VP argp)
{
    struct {
	ID id;
#ifdef notdef
	VP start;
#else
	ID rid;
#endif
	VP stat;
    } *args = argp;
#ifdef notdef
    return vsts_reg(args->id, args->start, args->stat);
#else
    return vsts_reg(args->id, args->rid, args->stat);
#endif
}



/*
 *
 */
static ER sys_vget_phs(VP argp)
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

static ER sys_set_tim(VP argp)
{
    struct {
	SYSTIME *pk_tim;
    } *args = argp;

    return (set_tim(args->pk_tim));
}

static ER sys_get_tim(VP argp)
{
    struct {
	SYSTIME *pk_tim;
    } *args = argp;

    return (get_tim(args->pk_tim));
}

static ER sys_dly_tsk(VP argp)
{
    struct {
	DLYTIME dlytim;
    } *args = argp;

    return (dly_tsk(args->dlytim));
}

static ER sys_def_alm(VP argp)
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
sys_vsys_inf (VP argp)
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
sys_dbg_puts (VP args)
{
  struct a
    {
      B *msg;
    } *arg = (struct a *)args;
  
  printk ("%s", arg->msg);
  return (E_OK);
}

/* sys_vsys_msc - miscellaneous function
 *
 */
static ER sys_vsys_msc(VP argp)
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
	dis_int();
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
	/* print_task_list */
	print_task_list();
	break;

    case 7:
	/* pmemstat */
	pmemstat();
	break;
    }

    return (E_OK);
}

/* sys_vcpy_stk - copy task stack 
 *
 */
static ER sys_vcpy_stk(VP argp)
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

/* sys_vset_cxt - set task context
 *
 */
static ER sys_vset_ctx(VP argp)
{
    struct {
	ID tid;
	W eip;
	B *stackp;
	W stsize;
    } *args = argp;

    return vset_ctx(args->tid, args->eip, args->stackp, args->stsize);
}

/* sys_vuse_fpu - use FPU
 *
 */
static ER sys_vuse_fpu(VP argp)
{
    struct {
	ID tid;
    } *args = argp;

    return vuse_fpu(args->tid);
}
