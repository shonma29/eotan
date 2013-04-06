/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#include <core.h>
#include <mpu/config.h>
#include <lowlib.h>
#include "func.h"
#include "misc.h"
#include "sync.h"
#include "mpu/mpufunc.h"

static ER make_task_stack(T_TCB * task, VP * sp);
static void make_local_stack(T_TCB * tsk, W size, W acc);


/* タスク情報を生成する:
 *
 *	引数：
 *		task		タスクのTCB領域へのポインタ
 *		stack_size	タスクのスタックサイズ
 *
 */
ER make_task_context(T_TCB * task, T_CTSK * pk_ctsk)
{
    VP sp;
    ER err;

    if (pk_ctsk->stksz > PAGE_SIZE) {
	return E_PAR;
    }

    err = make_task_stack(task, &sp);
    if (err != E_OK) {
	return (err);
    }

    /* スタック情報の登録 */
    task->stksz0 = task->stksz = PAGE_SIZE;
    task->stackptr0 = task->stackptr = (B *) sp;

    /* レジスタ類をすべて初期化する:
     * reset_registers()  は、以下の引数を必要とする：
     *   1) TCB 領域へのポインタ
     *   2) タスクのスタートアドレス
     *   3) カーネルスタックのアドレス
     */
    task->mpu.context.cs = KERNEL_CSEG;
    task->mpu.context.ds = KERNEL_DSEG;
    task->mpu.context.es = KERNEL_DSEG;
    task->mpu.context.fs = KERNEL_DSEG;
    task->mpu.context.gs = KERNEL_DSEG;
    task->mpu.context.ss = KERNEL_DSEG;
    task->mpu.context.ss0 = KERNEL_DSEG;
    task->mpu.context.esp = (UW) ((sp + pk_ctsk->stksz));
    task->mpu.context.ebp = (UW) ((sp + pk_ctsk->stksz));
    task->initial_stack = task->mpu.context.esp;
    task->mpu.context.eip = (UW) pk_ctsk->startaddr;
#ifdef TSKSW_DEBUG
    printk("(UW)pk_ctsk->startaddr = 0x%x\n", (UW) pk_ctsk->startaddr);
#endif
    task->mpu.context.eflags = EFLAG_IBIT | EFLAG_IOPL3;
    /* cre_tsk の中で bzero によって初期化される．
       task->context.eax = 0;
       task->context.ebx = 0;
       task->context.ecx = 0;
       task->context.esi = 0;
       task->context.edi = 0;
       task->context.zero = 0;
       task->context.t = 0;
       task->context.iobitmap = 0;
     */
    task->mpu.tss_selector = ((TSS_BASE + task->tskid) << 3) & 0xfff8;
    task->mpu.use_fpu = 0;		/* 初期状態では FPU は利用しない */

    create_context(task);	/* コンテキスト領域(TSS)のアドレスをGDTにセット */
    return (E_OK);		/* set_task_registers (task, pk_ctsk->startaddr, sp)); */
}

/* make_task_stack --- タスクスタックを生成する。
 *
 * カーネルモードで使用するタスク用スタックを生成する。
 *
 */
static ER make_task_stack(T_TCB * task, VP * sp)
{
    /* スタックポインタは 0x80000000 の仮想アドレスでアクセスする必要がある。 */
    (*sp) = (VP*)((UW)palloc(1) | MIN_KERNEL);
#ifdef TSKSW_DEBUG
    printk("sp = 0x%x\n", *sp);
#endif
    if (*sp == (VP) NULL) {
#ifdef TSKSW_DEBUG
	printk("make_task_stack: palloc fail.\n");
#endif
	return (E_NOMEM);
    }

    return (E_OK);
}

/*
 * make_local_stack
 */
static void make_local_stack(T_TCB * tsk, W size, W acc)
{
    W err;

    /* task の kernel stack 領域を特権レベル 0 のスタックに設定 */
    tsk->mpu.context.esp0 = tsk->initial_stack;

    /* stack region の作成 number は 4 で固定 */
    /* VM_READ, VM_WRITE, VM_USER) is originally defined in posix_mm.h */
#define VM_READ		0x00000001
#define VM_WRITE	0x00000002
#define VM_USER		0x00010000
    region_create(tsk->tskid, STACK_REGION,
	     (VP) VADDR_STACK_HEAD, STD_STACK_SIZE, STD_STACK_SIZE,
	     (VM_READ | VM_WRITE | VM_USER), NULL);

    /* 物理メモリの割り当て */
    tsk->stackptr = (VP) (VADDR_STACK_TAIL - size);
    tsk->stksz = size;
    tsk->initial_stack = VADDR_STACK_TAIL;
    err = region_map(tsk->tskid, (VP) tsk->stackptr, size, acc);

    if (err != E_OK) {
	printk("can't allocate stack\n");
    }
}

/*
 * mpu_copy_stack
 */
ER mpu_copy_stack(ID src, W esp, W ebp, W ebx, W ecx, W edx, W esi, W edi, ID dst)
{
    T_TCB *src_tsk, *dst_tsk;
    UW srcp, dstp;
    UW size;

    src_tsk = get_thread_ptr(src);

    if (!src_tsk) {
	return (E_ID);
    }

    enter_critical();
    dst_tsk = get_thread_ptr(dst);

    /* dst task に新しいスタックポインタを割り付ける */
    make_local_stack(dst_tsk, POSIX_STACK_SIZE, ACC_USER);

    size = ((UW) src_tsk->stackptr) + src_tsk->stksz - ebp;
    dstp = ((UW) dst_tsk->stackptr) + dst_tsk->stksz - size;
    dst_tsk->mpu.context.ebp = dstp;

    size = ((UW) src_tsk->stackptr) + src_tsk->stksz - esp;
    dstp = ((UW) dst_tsk->stackptr) + dst_tsk->stksz - size;
    dst_tsk->mpu.context.esp = dstp;

    /* src task のスタックの内容を dst task にコピー */
    srcp = (UW) src_tsk->initial_stack;
    dstp = (UW) dst_tsk->initial_stack;
    while(size >= PAGE_SIZE) {
      srcp -= PAGE_SIZE;
      dstp -= PAGE_SIZE;
      region_put(dst, (VP) dstp, PAGE_SIZE, (VP) vtor(src_tsk->tskid, srcp));
      size -= PAGE_SIZE;
    }
    if (size > 0) {
      srcp -= size;
      dstp -= size;
      region_put(dst, (VP) dstp, size, (VP) vtor(src_tsk->tskid, srcp));
    }

    /* レジスタのコピー */
    dst_tsk->mpu.context.ebx = ebx;
    dst_tsk->mpu.context.ecx = ecx;
    dst_tsk->mpu.context.edx = edx;
    dst_tsk->mpu.context.esi = esi;
    dst_tsk->mpu.context.edi = edi;

    /* セレクタの設定 */
    dst_tsk->mpu.context.cs = USER_CSEG | USER_DPL;
    dst_tsk->mpu.context.ds = USER_DSEG;
    dst_tsk->mpu.context.es = USER_DSEG;
    dst_tsk->mpu.context.fs = USER_DSEG;
    dst_tsk->mpu.context.gs = USER_DSEG;
    dst_tsk->mpu.context.ss = USER_SSEG | USER_DPL;

    /* FPU 情報のコピー */
    dst_tsk->mpu.use_fpu = src_tsk->mpu.use_fpu;

    leave_critical();
    return (E_OK);
}

/* default page fault handler */
W pf_handler(W cr2, W eip)
{
    /* KERNEL_TASK への登録 */
    /* type = POSIX, id = pid */
    add_trmtbl(0, run_task->tskid, (LOWLIB_DATA)->my_pid);
    /* KERNEL_TASK の優先度変更 */
    thread_change_priority(KERNEL_TASK, MID_LEVEL);
    return (E_OK);
}

/* mpu_set_context */
ER mpu_set_context(ID tid, W eip, B * stackp, W stsize)
{
    T_TCB *tsk = get_thread_ptr(tid);
    UW stbase;
    W err, argc;
    char **ap, **bp, **esp;

    if (!tsk) {
	return (E_ID);
    }

    enter_critical();

    /* stack frame の作成． */
    /* stack のサイズが PAGE_SIZE (4KB) を越えると問題が発生する可能性あり */
    /* これに対応するには palloc で割り当てたメモリに stack frame を作成し */
    /* vput_reg する */
    if (stsize >= PAGE_SIZE) {
	printk("WARNING vset_ctx: stack size is too large\n");
    }

    stbase = tsk->initial_stack - ROUNDUP(stsize, sizeof(VP));
    esp = (char **) stbase;
    ap = bp = (char **) vtor(tsk->tskid, stbase);

    err = region_get(tid, stackp, stsize, (VP) ap);
    if (err)
	return err;

    for (argc = 0; *ap != 0; ++ap, ++argc) {
	*ap = (char *) ((UW) * ap + stbase);
    }
    ++ap;
    *--bp = (char *) (stbase + sizeof(VP) * (argc+1));
    --esp;

    for (; *ap != 0; ++ap) {
	*ap = (char *) ((UW) * ap + stbase);
    }
    *--bp = (char *) stbase;
    --esp;
    *--bp = (char *) argc;
    --esp;
    *--bp = NULL;
    --esp;

    tsk->mpu.context.esp = (UW) esp;
    tsk->mpu.context.ebp = (UW) esp;

    /* レジスターの初期化 */
    tsk->mpu.context.eip = eip;

    tsk->mpu.context.eflags = EFLAG_IBIT | EFLAG_IOPL3;
    tsk->mpu.context.eax = 0;
    tsk->mpu.context.ebx = 0;
    tsk->mpu.context.ecx = 0;
    tsk->mpu.context.edx = 0;
    tsk->mpu.context.esi = 0;
    tsk->mpu.context.edi = 0;
    tsk->mpu.context.t = 0;

    /* セレクタの設定 */
    tsk->mpu.context.cs = USER_CSEG | USER_DPL;
    tsk->mpu.context.ds = USER_DSEG;
    tsk->mpu.context.es = USER_DSEG;
    tsk->mpu.context.fs = USER_DSEG;
    tsk->mpu.context.gs = USER_DSEG;
    tsk->mpu.context.ss = USER_SSEG | USER_DPL;

    /* タスクの初期化 */

    /* page fault handler の登録 */
    tsk->page_fault_handler = pf_handler;

    /* quantum の設定 */
    tsk->quantum = QUANTUM;
    leave_critical();

    list_remove(&(tsk->wait.waiting));
    release(tsk);
    return (E_OK);
}

/* mpu_use_float */
ER mpu_use_float(ID tid)
{
    T_TCB *taskp = get_thread_ptr(tid);

    if (!taskp) {
	return (E_ID);
    }
    fpu_start(taskp);
    return (E_OK);
}

void set_autorun_context(T_TCB *taskp) {
   /* 固有のスタックを用意 */
   make_local_stack(taskp, KERNEL_STACK_SIZE, ACC_USER);
   taskp->mpu.context.esp = taskp->initial_stack;
   taskp->mpu.context.ebp = taskp->initial_stack;

   /* セレクタの設定 */
   taskp->mpu.context.cs = USER_CSEG | USER_DPL;
   taskp->mpu.context.ds = USER_DSEG;
   taskp->mpu.context.es = USER_DSEG;
   taskp->mpu.context.fs = USER_DSEG;
   taskp->mpu.context.gs = USER_DSEG;
   taskp->mpu.context.ss = USER_SSEG | USER_DPL;
}

void set_thread1_context(T_TCB *taskp) {
    /* タスク 1 のコンテキスト情報を初期化する                    */
    /* これらの情報は、次にタスク1がカレントタスクになった時に    */
    /* 使用する                                                   */
    taskp->mpu.context.cr3 = (UW) PAGE_DIR_ADDR;
    taskp->mpu.context.cs = KERNEL_CSEG;
    taskp->mpu.context.ds = KERNEL_DSEG;
    taskp->mpu.context.es = KERNEL_DSEG;
    taskp->mpu.context.fs = KERNEL_DSEG;
    taskp->mpu.context.gs = KERNEL_DSEG;
    taskp->mpu.context.ss = KERNEL_DSEG;
    taskp->mpu.context.ss0 = KERNEL_DSEG;
    taskp->mpu.context.eflags = EFLAG_IBIT | EFLAG_IOPL3;
}

void set_thread1_start(T_TCB *taskp) {
    /* セレクタをセット */
    taskp->mpu.tss_selector =
	((KERNEL_TASK + TSS_BASE) << 3) & 0xfff8;
    create_context(taskp);

    /* タスクレジスタの値を設定する. */
    load_task_register((KERNEL_TASK + TSS_BASE) << 3);
}

void set_page_table(T_TCB *taskp, UW p) {
    taskp->mpu.context.cr3 = p;
}

void set_sp(T_TCB *taskp, UW p) {
    taskp->mpu.context.esp -= p;
}

