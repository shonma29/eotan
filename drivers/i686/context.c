/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/

#include <core.h>
#include <boot/init.h>
#include <local.h>
#include <vm.h>
#include <mpu/config.h>
#include "func.h"
#include "sync.h"
#include "mpu/mpufunc.h"
#include "mpu/msr.h"

static ER allocate_kernel_stack(T_TCB * task, VP * sp);
static void create_stack(T_TCB * tsk, W size, W acc);
static void set_user_registers(T_TCB *taskp);


/* タスク情報を生成する:
 *
 *	引数：
 *		task		タスクのTCB領域へのポインタ
 *		stack_size	タスクのスタックサイズ
 *
 */
ER create_context(T_TCB * task, T_CTSK * pk_ctsk)
{
    VP sp;
    ER err;

    if (pk_ctsk->stksz > PAGE_SIZE) {
	return E_PAR;
    }

    err = allocate_kernel_stack(task, &sp);
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
    set_kthread_registers(task);
    task->mpu.context.esp = (UW) ((sp + pk_ctsk->stksz));
    task->mpu.context.ebp = (UW) ((sp + pk_ctsk->stksz));
    task->initial_stack = task->mpu.context.esp;
    task->mpu.context.eip = (UW) pk_ctsk->task;
#ifdef TSKSW_DEBUG
    printk("(UW)pk_ctsk->startaddr = 0x%x\n", (UW) pk_ctsk->startaddr);
#endif
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
    task->mpu.use_fpu = 0;		/* 初期状態では FPU は利用しない */

    create_tss(task);	/* コンテキスト領域(TSS)のアドレスをGDTにセット */
    return (E_OK);		/* set_task_registers (task, pk_ctsk->startaddr, sp)); */
}

/* allocate_kernel_stack --- タスクスタックを生成する。
 *
 * カーネルモードで使用するタスク用スタックを生成する。
 *
 */
static ER allocate_kernel_stack(T_TCB * task, VP * sp)
{
    /* スタックポインタは 0x80000000 の仮想アドレスでアクセスする必要がある。 */
    (*sp) = kern_p2v(palloc(1));
#ifdef TSKSW_DEBUG
    printk("sp = 0x%x\n", *sp);
#endif
    if (*sp == (VP) NULL) {
#ifdef TSKSW_DEBUG
	printk("allocate_kernel_stack: palloc fail.\n");
#endif
	return (E_NOMEM);
    }

    return (E_OK);
}

/*
 * create_stack
 */
static void create_stack(T_TCB * tsk, W size, W acc)
{
    W err;

    /* task の kernel stack 領域を特権レベル 0 のスタックに設定 */
    tsk->mpu.context.esp0 = tsk->initial_stack;

    /* stack region の作成 number は 4 で固定 */
    region_create(tsk->tskid, STACK_REGION,
	     (VP) VADDR_STACK_HEAD, STD_STACK_SIZE, STD_STACK_SIZE,
	     (VM_READ | VM_WRITE | VM_USER));

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
ER mpu_copy_stack(ID src, W esp, ID dst)
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
    create_stack(dst_tsk, USER_STACK_SIZE, ACC_USER);

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

    /* セレクタの設定 */
    set_user_registers(dst_tsk);

    /* FPU 情報のコピー */
    dst_tsk->mpu.use_fpu = src_tsk->mpu.use_fpu;

    leave_critical();
    return (E_OK);
}

/* default page fault handler */
W pf_handler(W cr2, W eip)
{
    thread_local_t *local = (thread_local_t*)LOCAL_ADDR;

    /* KERNEL_TASK への登録 */
    /* type = POSIX, id = pid */
    add_trmtbl(0, run_task->tskid, local->process_id);
    /* KERNEL_TASK の優先度変更 */
    thread_change_priority(KERNEL_TASK, MID_LEVEL);
    return (E_OK);
}

/* mpu_set_context */
ER mpu_set_context(ID tid, W eip, B * stackp, W stsize)
{
    T_TCB *tsk = get_thread_ptr(tid & INIT_THREAD_ID_MASK);
    UW stbase;
    W err, argc;
    char **ap, **bp, **esp;

    if (!tsk) {
	return (E_ID);
    }

    if (tid & INIT_THREAD_ID_FLAG) {
	tid &= INIT_THREAD_ID_MASK;
	create_stack(tsk, USER_STACK_SIZE, ACC_USER);
        fpu_start(tsk);
    }

    enter_critical();

    /* stack frame の作成． */
    /* stack のサイズが PAGE_SIZE (4KB) を越えると問題が発生する可能性あり */
    /* これに対応するには palloc で割り当てたメモリに stack frame を作成し */
    /* vput_reg する */
    if (stsize >= PAGE_SIZE) {
	printk("WARNING vset_ctx: stack size is too large\n");
    }

    stbase = tsk->initial_stack
	    - roundUp(stsize, sizeof(VP));
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
    set_user_registers(tsk);

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

void set_kthread_registers(T_TCB *taskp)
{
    taskp->mpu.context.cs = kern_code;
    taskp->mpu.context.ds = kern_data;
    taskp->mpu.context.es = kern_data;
    taskp->mpu.context.fs = kern_data;
    taskp->mpu.context.gs = kern_data;
    taskp->mpu.context.ss = kern_data;
    taskp->mpu.context.ss0 = kern_data;
    taskp->mpu.context.eflags = EFLAG_IBIT | EFLAG_IOPL3;
}

static void set_user_registers(T_TCB *taskp)
{
    taskp->mpu.context.cs = user_code | USER_DPL;
    taskp->mpu.context.ds = user_data;
    taskp->mpu.context.es = user_data;
    taskp->mpu.context.fs = user_data;
    taskp->mpu.context.gs = user_data;
    taskp->mpu.context.ss = user_data | USER_DPL;
}

void start_thread1(T_TCB *taskp)
{
    /* セレクタをセット */
    create_tss(taskp);

    /* タスクレジスタの値を設定する. */
    tr_set((taskp->tskid + TSS_BASE) << 3);
}

void set_page_table(T_TCB *taskp, UW p)
{
    taskp->mpu.context.cr3 = p;
}

void set_sp(T_TCB *taskp, UW p)
{
    taskp->mpu.context.esp -= p;
}

void prepare_kernel_sp(T_TCB *taskp)
{
    msr_write(sysenter_esp_msr, taskp->mpu.context.esp0);
    tss_set_kernel_sp((VP)(taskp->mpu.context.esp0));
}
