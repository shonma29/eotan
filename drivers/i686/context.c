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

static void create_user_stack(thread_t * tsk, W size, W acc);


/*
 * create_user_stack
 */
static void create_user_stack(thread_t * tsk, W size, W acc)
{
    W err;

    /* stack region の作成 number は 3 で固定 */
    region_create(thread_id(tsk), STACK_REGION,
	     (VP) VADDR_STACK_HEAD, STD_STACK_SIZE, STD_STACK_SIZE,
	     (VM_READ | VM_WRITE | VM_USER));

    /* 物理メモリの割り当て */
    tsk->attr.ustack.addr = (VP) (VADDR_STACK_TAIL - size);
    tsk->attr.ustack.length = size;
    tsk->attr.ustack_top = (VP)VADDR_STACK_TAIL;
    err = region_map(thread_id(tsk), (VP) tsk->attr.ustack.addr, size, acc);

    if (err != E_OK) {
	printk("can't allocate stack\n");
    }
}

/*
 * mpu_copy_stack
 */
ER mpu_copy_stack(ID src, W esp, ID dst)
{
    thread_t *src_tsk, *dst_tsk;
    UW srcp, dstp;
    UW size;
    UW ustack_top;

    src_tsk = get_thread_ptr(src);

    if (!src_tsk) {
	return (E_NOEXS);
    }

    enter_critical();
    dst_tsk = get_thread_ptr(dst);

    /* dst task に新しいスタックポインタを割り付ける */
    create_user_stack(dst_tsk, USER_STACK_SIZE, ACC_USER);

    size = ((UW) src_tsk->attr.ustack.addr) + src_tsk->attr.ustack.length - esp;
    ustack_top = ((UW) dst_tsk->attr.ustack.addr) + dst_tsk->attr.ustack.length - size;

    /* src task のスタックの内容を dst task にコピー */
    srcp = (UW) src_tsk->attr.ustack_top;
    dstp = (UW) dst_tsk->attr.ustack_top;
    while(size >= PAGE_SIZE) {
      srcp -= PAGE_SIZE;
      dstp -= PAGE_SIZE;
      region_put(dst, (VP) dstp, PAGE_SIZE, (VP) vtor(thread_id(src_tsk), srcp));
      size -= PAGE_SIZE;
    }
    if (size > 0) {
      srcp -= size;
      dstp -= size;
      region_put(dst, (VP) dstp, size, (VP) vtor(thread_id(src_tsk), srcp));
    }

    dst_tsk->mpu.context.esp0 = context_create_user(
	    dst_tsk->attr.kstack_top,
	    EFLAG_IBIT | EFLAG_IOPL3,
	    dst_tsk->attr.entry,
	    (VP)ustack_top);

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
    /* id = pid */
    if (lfq_enqueue(&kqueue, &(local->process_id)) != QUEUE_OK)
    	panic("full kqueue");

    thread_start(delay_thread_id);
    return (E_OK);
}

/* mpu_set_context */
ER mpu_set_context(ID tid, W eip, B * stackp, W stsize)
{
    thread_t *tsk = get_thread_ptr(tid & INIT_THREAD_ID_MASK);
    UW stbase;
    W err, argc;
    char **ap, **bp, **esp;

    if (!tsk) {
	return (E_NOEXS);
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

    stbase = (UW)tsk->attr.ustack_top
	    - roundUp(stsize, sizeof(VP));
    esp = (char **) stbase;
    ap = bp = (char **) vtor(thread_id(tsk), stbase);

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

    /* レジスターの初期化 */
    tsk->attr.entry = (FP)eip;

    /* タスクの初期化 */
    tsk->mpu.context.esp0 = context_create_user(
	    tsk->attr.kstack_top,
	    EFLAG_IBIT | EFLAG_IOPL3,
	    tsk->attr.entry,
	    (VP)esp);

    /* page fault handler の登録 */
    tsk->handler = pf_handler;

    leave_critical();

    list_remove(&(tsk->wait.waiting));
    release(tsk);
    return (E_OK);
}

void set_page_table(thread_t *taskp, VP p)
{
    taskp->mpu.context.cr3 = p;
}

void set_arg(thread_t *taskp, const UW arg)
{
    UW *sp = taskp->mpu.context.esp0;

    *--sp = arg;
    taskp->mpu.context.esp0 = sp;
}