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
#include <mm/segment.h>
#include <nerve/config.h>
#include "delay.h"
#include "func.h"
#include "ready.h"
#include "sync.h"
#include "mpu/eflags.h"
#include "mpu/interrupt.h"
#include "mpu/mpufunc.h"

static void create_user_stack(thread_t * tsk);
static void kill(void);


/*
 * create_user_stack
 */
static void create_user_stack(thread_t * tsk)
{
    W err;

    tsk->ustack.addr = (VP) pageRoundDown(LOCAL_ADDR - USER_STACK_MAX_SIZE);
    tsk->ustack.len = pageRoundUp(USER_STACK_INITIAL_SIZE);
    tsk->ustack.max = pageRoundUp(USER_STACK_MAX_SIZE - PAGE_SIZE);
    tsk->ustack.attr = type_stack;

    /* 物理メモリの割り当て */
    tsk->attr.ustack_tail = (VP)((UW)(tsk->ustack.addr) + tsk->ustack.max);
    err = region_map(tsk->attr.page_table,
    	    (VP)((UW)(tsk->attr.ustack_tail) - tsk->ustack.len),
	    tsk->ustack.len, true);

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
    UW dstp;
    UW size;

    src_tsk = get_thread_ptr(src);

    if (!src_tsk) {
	return (E_NOEXS);
    }

    dst_tsk = get_thread_ptr(dst);
    if (!dst_tsk) {
	return (E_NOEXS);
    }

    enter_critical();

    /* dst task に新しいスタックポインタを割り付ける */
    create_user_stack(dst_tsk);

    size = ((UW) src_tsk->attr.ustack_tail) - esp;

    /* src task のスタックの内容を dst task にコピー */
    dstp = (UW) dst_tsk->attr.ustack_tail - size;
    region_put(dst, (VP) dstp, size, (VP) vtor(thread_id(src_tsk), esp));

    dst_tsk->mpu.esp0 = context_create_user(
	    dst_tsk->attr.kstack_tail,
	    EFLAGS_INTERRUPT_ENABLE | EFLAGS_IOPL_3,
	    dst_tsk->attr.entry,
	    (VP)dstp);

    leave_critical();
    return (E_OK);
}

static void kill(void)
{
    thread_local_t *local = (thread_local_t*)LOCAL_ADDR;
    delay_param_t param;

    /* DELAY_TASK への登録 */
    param.action = delay_page_fault;
    /* id = pid */
    param.arg1 = (int)(local->process_id);

    if (kq_enqueue(&param))
    	panic("full kqueue");
}

/* default page fault handler */
ER context_page_fault_handler(void)
{
    UW addr;

    if (is_kthread(running)) {
    	return (E_SYS);
    }

    addr = (UW)fault_get_addr();

    /* フォルトを起こしたアドレスがスタック領域にあればページを割り当てる */
    if (running->ustack.attr &&
	    (((UW) running->ustack.addr <= addr) &&
	    (addr <= ((UW) running->ustack.addr + running->ustack.max)))) {
	ER result = region_map(running->attr.page_table, (VP) addr, PAGE_SIZE, true);

	if (result == E_OK) {
	    /* ページフォルト処理に成功した */
	    tlb_flush_all();
	    return (E_OK);
	}
    }

    kill();
    return (E_OK);
}

ER context_mpu_handler(void)
{
    if (is_kthread(running)) {
    	return (E_SYS);
    }

    kill();
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
	create_user_stack(tsk);
    }

    enter_critical();

    /* stack frame の作成． */
    /* stack のサイズが USER_STACK_INITIAL_SIZE を越えると問題が発生する可能性あり */
    /* これに対応するには palloc で割り当てたメモリに stack frame を作成し */
    /* vput_reg する */
    if (stsize >= USER_STACK_INITIAL_SIZE) {
	printk("WARNING mpu_set_context: stack size is too large\n");
    }

    stbase = (UW)tsk->attr.ustack_tail
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
    tsk->mpu.esp0 = context_create_user(
	    tsk->attr.kstack_tail,
	    EFLAGS_INTERRUPT_ENABLE | EFLAGS_IOPL_3,
	    tsk->attr.entry,
	    (VP)esp);

    leave_critical();

    return (E_OK);
}
