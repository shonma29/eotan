/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#include "core.h"
#include "func.h"
#include "lowlib.h"
#include "misc.h"
#include "sync.h"
#include "mpu/mpufunc.h"

static ER make_task_stack(T_TCB * task, W size, VP * sp);


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

    err = make_task_stack(task, pk_ctsk->stksz, &sp);
    if (err != E_OK) {
	return (err);
    }

    /* スタック情報の登録 */
    task->stksz0 = task->stksz = pk_ctsk->stksz;
    task->stackptr0 = task->stackptr = (B *) sp;

    /* レジスタ類をすべて初期化する:
     * reset_registers()  は、以下の引数を必要とする：
     *   1) TCB 領域へのポインタ
     *   2) タスクのスタートアドレス
     *   3) カーネルスタックのアドレス
     */
    task->context.cs = KERNEL_CSEG;
    task->context.ds = KERNEL_DSEG;
    task->context.es = KERNEL_DSEG;
    task->context.fs = KERNEL_DSEG;
    task->context.gs = KERNEL_DSEG;
    task->context.ss = KERNEL_DSEG;
    task->context.ss0 = KERNEL_DSEG;
    task->context.esp = (UW) ((sp + pk_ctsk->stksz));
    task->context.ebp = (UW) ((sp + pk_ctsk->stksz));
    task->initial_stack = task->context.esp;
    task->context.eip = (UW) pk_ctsk->startaddr;
#ifdef TSKSW_DEBUG
    printk("(UW)pk_ctsk->startaddr = 0x%x\n", (UW) pk_ctsk->startaddr);
#endif
    task->context.eflags = EFLAG_IBIT | EFLAG_IOPL3;
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
    task->tss_selector = ((TSS_BASE + task->tskid) << 3) & 0xfff8;
    task->use_fpu = 0;		/* 初期状態では FPU は利用しない */

    create_context(task);	/* コンテキスト領域(TSS)のアドレスをGDTにセット */
    return (E_OK);		/* set_task_registers (task, pk_ctsk->startaddr, sp)); */
}

/* make_task_stack --- タスクスタックを生成する。
 *
 * カーネルモードで使用するタスク用スタックを生成する。
 *
 */
static ER make_task_stack(T_TCB * task, W size, VP * sp)
{
    /* スタックポインタは 0x80000000 の仮想アドレスでアクセスする必要がある。 */
    (*sp) = (VP*)((UW)palloc(PAGES(size)) | 0x80000000);
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
void make_local_stack(T_TCB * tsk, W size, W acc)
{
    W err;

    /* task の kernel stack 領域を特権レベル 0 のスタックに設定 */
    tsk->context.esp0 = tsk->initial_stack;

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
	printk("[KERN] can't allocate stack\n");
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
    dst_tsk->context.ebp = dstp;

    size = ((UW) src_tsk->stackptr) + src_tsk->stksz - esp;
    dstp = ((UW) dst_tsk->stackptr) + dst_tsk->stksz - size;
    dst_tsk->context.esp = dstp;

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
    dst_tsk->context.ebx = ebx;
    dst_tsk->context.ecx = ecx;
    dst_tsk->context.edx = edx;
    dst_tsk->context.esi = esi;
    dst_tsk->context.edi = edi;

    /* セレクタの設定 */
    dst_tsk->context.cs = USER_CSEG | USER_DPL;
    dst_tsk->context.ds = USER_DSEG;
    dst_tsk->context.es = USER_DSEG;
    dst_tsk->context.fs = USER_DSEG;
    dst_tsk->context.gs = USER_DSEG;
    dst_tsk->context.ss = USER_SSEG | USER_DPL;

    /* FPU 情報のコピー */
    dst_tsk->use_fpu = src_tsk->use_fpu;

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
	printk("[KERN] WARNING vset_ctx: stack size is too large\n");
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

    tsk->context.esp = (UW) esp;
    tsk->context.ebp = (UW) esp;

    /* レジスターの初期化 */
    tsk->context.eip = eip;

    tsk->context.eflags = EFLAG_IBIT | EFLAG_IOPL3;
    tsk->context.eax = 0;
    tsk->context.ebx = 0;
    tsk->context.ecx = 0;
    tsk->context.edx = 0;
    tsk->context.esi = 0;
    tsk->context.edi = 0;
    tsk->context.t = 0;

    /* セレクタの設定 */
    tsk->context.cs = USER_CSEG | USER_DPL;
    tsk->context.ds = USER_DSEG;
    tsk->context.es = USER_DSEG;
    tsk->context.fs = USER_DSEG;
    tsk->context.gs = USER_DSEG;
    tsk->context.ss = USER_SSEG | USER_DPL;

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
