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
#include "setting.h"
#include "sync.h"
#include "mpu/mpufunc.h"

static ER allocate_kernel_stack(T_TCB * task, VP * sp);
static void create_user_stack(T_TCB * tsk, W size, W acc);


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
    task->mpu.context.esp = (UW) ((sp + pk_ctsk->stksz));
    task->initial_stack = task->mpu.context.esp;
    task->mpu.context.eip = (UW) pk_ctsk->task;
#ifdef TSKSW_DEBUG
    printk("(UW)pk_ctsk->task = 0x%x\n", (UW) pk_ctsk->task);
#endif
    task->mpu.use_fpu = 0;		/* 初期状態では FPU は利用しない */

    if (pk_ctsk->exinf == KERNEL_DOMAIN_ID) {
	task->stacktop0 = context_create_kernel(
		(VP_INT*)(task->mpu.context.esp),
		EFLAG_IBIT | EFLAG_IOPL3,
		(FP)(task->mpu.context.eip));
    }

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
 * create_user_stack
 */
static void create_user_stack(T_TCB * tsk, W size, W acc)
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
    create_user_stack(dst_tsk, USER_STACK_SIZE, ACC_USER);

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

    dst_tsk->stacktop0 = context_create_user(
	    (VP_INT*)(dst_tsk->mpu.context.esp0),
	    EFLAG_IBIT | EFLAG_IOPL3,
	    (FP)(dst_tsk->mpu.context.eip),
	    (VP_INT*)(dst_tsk->mpu.context.esp));

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
    thread_change_priority(KERNEL_TASK, pri_dispatcher);
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
	create_user_stack(tsk, USER_STACK_SIZE, ACC_USER);
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

    /* レジスターの初期化 */
    tsk->mpu.context.eip = eip;

    /* タスクの初期化 */
    tsk->stacktop0 = context_create_user(
	    (VP_INT*)(tsk->mpu.context.esp0),
	    EFLAG_IBIT | EFLAG_IOPL3,
	    (FP)(tsk->mpu.context.eip),
	    (VP_INT*)(tsk->mpu.context.esp));

    /* page fault handler の登録 */
    tsk->page_fault_handler = pf_handler;

    /* quantum の設定 */
    tsk->quantum = QUANTUM;
    leave_critical();

    list_remove(&(tsk->wait.waiting));
    release(tsk);
    return (E_OK);
}

void set_page_table(T_TCB *taskp, VP p)
{
    taskp->mpu.context.cr3 = (UW)p;
}

void set_arg(T_TCB *taskp, const UW arg)
{
    UW *sp = (UW*)(taskp->mpu.context.esp);

    *--sp = arg;
    taskp->mpu.context.esp = (UW)sp;
}
