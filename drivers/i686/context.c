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
#include <mpu/memory.h>
#include <nerve/config.h>
#include <delay.h>
#include <func.h>
#include <ready.h>
#include <sync.h>
#include <thread.h>
#include "eflags.h"
#include "interrupt.h"
#include "mpufunc.h"

static void kill(void);
static ER region_map(VP page_table, VP start, UW size, W accmode);


/*
 * create_user_stack
 */
void create_user_stack(thread_t * tsk)
{
    tsk->ustack.addr = (VP) pageRoundDown(LOCAL_ADDR - USER_STACK_MAX_SIZE);
    tsk->ustack.len = pageRoundUp(USER_STACK_INITIAL_SIZE);
    tsk->ustack.max = pageRoundUp(USER_STACK_MAX_SIZE - PAGE_SIZE);
    tsk->ustack.attr = type_stack;

    tsk->attr.ustack_tail = (VP)((UW)(tsk->ustack.addr) + tsk->ustack.max);
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

/*
 * リージョン内の仮想ページへ物理メモリを割り付ける。
 *
 * 引数で指定したアドレス領域に物理メモリを割り付ける。
 *
 * 複数のページに相当するサイズが指定された場合、全てのページがマップ
 * 可能のときのみ物理メモリを割り付ける。その他の場合は割り付けない。
 *
 * マップする物理メモリのアドレスは指定できない。中心核が仮想メモリに
 * 割り付ける物理メモリを適当に割り振る。
 *
 *
 * 返り値
 *
 * 以下のエラー番号が返る。
 *	E_OK     リージョンのマップに成功  
 *	E_NOMEM  (物理)メモリが不足している
 *	E_NOSPT  本システムコールは、未サポート機能である。
 *	E_PAR	 引数がおかしい
 *
 */
static ER region_map(VP page_table, VP start, UW size, W accmode)
    /* 
     * page_table        仮想メモリマップ
     * start     マップする仮想メモリ領域の先頭アドレス
     * size      マップする仮想メモリ領域の大きさ(バイト単位)
     * accmode   マップする仮想メモリ領域のアクセス権を指定
     *           (ACC_KERNEL = 0, ACC_USER = 1)
     */
{
    ER res;

printk("region_map: %x %p %x %x\n", page_table, start, size, accmode);
    size = pages(size);
    start = (VP)pageRoundDown((UW)start);
    if (pmemfree() < size)
	return (E_NOMEM);
    res = map_user_pages(page_table, start, size);
    if (res != E_OK) {
	unmap_user_pages(page_table, start, size);
    }
    return (res);
}
