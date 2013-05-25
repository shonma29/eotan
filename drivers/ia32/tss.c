/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/
/* tss.c

 TSS (Task State Segment) の管理をする。
 
 
 Interface:
 	creat_context:	コンテキスト情報を新しく作成する。
	delete_context:	コンテキスト情報を削除する。
	
 */


#include <core.h>
#include <mpu/config.h>
#include "thread.h"
#include "func.h"
#include "mpufunc.h"

static void
init_task_state_segment (W index, T_CONTEXT *tss);


/* creat_context コンテキスト情報を新しく作成する。
 *
 */
void
create_tss (T_TCB *task)
{
  T_CONTEXT	*tss;

  task->mpu.tss_selector = ((TSS_BASE + task->tskid) << 3) & 0xfff8;
  tss = &(task->mpu.context);
  init_task_state_segment (task->tskid, tss);
}

/* init_task_state_segment -- TSS を指すディスクリプタの初期化
 *
 * 引数：	index	初期化する task_state_seg テーブルのインデックス
 *
 * 返値：	なし
 *
 * 処理：	引数で指定しているタスクの TSS を指すためのディスクリプタを
 *		初期化する。
 *		TSS ディスプリタの形式を次に示す。
 *
 *	
 *
 */
static void
init_task_state_segment (W index, T_CONTEXT *tss)
{
  TASK_DESC *task_desc;

  task_desc = (TASK_DESC *)GDT_ADDR;
  task_desc = &task_desc[index + TSS_BASE];

#ifdef TSKSW_DEBUG
  printk ("init_task_state_segment: gdt[%d] = 0x%x\n", index + TSS_BASE);
#endif
  task_desc->zero0 = 0;
  task_desc->zero1 = 0;
  task_desc->dpl = 0;
  task_desc->present = 1;
  task_desc->type = TYPE_TSS;
  task_desc->g = 0;
  task_desc->d = 1;
  SET_TSS_ADDR (*task_desc, tss);
  SET_TSS_LIMIT (*task_desc, sizeof(T_CONTEXT));
#ifdef TSKSW_DEBUG
  print_task_desc (task_desc);
#endif
}

#ifdef TSKSW_DEBUG
void
print_task_desc (TASK_DESC *desc)
{
  printk ("** task descriptor **\n");
  printk ("sizeof TASK_DESC = %d\n", sizeof (*desc));
  printk ("tss addr = 0x%x\n", desc);
  printk ("   type  = %d\n", desc->type);
  printk ("   addr  = 0x%x\n", GET_TSS_ADDR(*desc));
  printk ("   limit = 0x%x\n", GET_TSS_LIMIT(*desc));
}
#endif

