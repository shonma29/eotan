/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* fault.c
 *
 *
 */

#include <core.h>
#include <string.h>
#include <mpu/config.h>
#include <mpu/io.h>
#include "../../kernel/arch/8259a.h"
#include "../../kernel/arch/archfunc.h"
#include "interrupt.h"
#include "thread.h"
#include "func.h"
#include "sync.h"
#include "mpufunc.h"
#include "gate.h"

/* T_INTR_HANDLER	割り込みハンドラ定義
 *
 */
typedef struct intr_handler_t {
    W masklevel;
    void (*handler) (VP sp);
} T_INTR_HANDLER;

/*
 *	割り込み処理の大域変数
 *
 */
volatile W on_interrupt = 0;
BOOL delayed_dispatch = FALSE;

/*
 * 割り込みハンドラテーブル
 * def_int システムコールで登録するときに使用する。
 */
struct intr_entry {
    ATR attr;
    FP func;
};

struct intr_entry intr_table[128];



/**************************************************************************
 * init_interrupt --- 割り込み機能の初期化を行う。
 *
 * 引数：なし
 *
 * 返値：エラー番号
 *
 */
W init_interrupt(void)
{
    W i;

    printk("init_interrupt\n");
    pic_initialize();

    idt_set(int_division_error, kern_code, handle0, interruptGate32, dpl_kern);
    idt_set(int_debugger, kern_code, handle1, interruptGate32, dpl_kern);
    idt_set(int_nmi, kern_code, handle2, interruptGate32, dpl_kern);
    idt_set(int_break_point, kern_code, handle3, interruptGate32, dpl_kern);
    idt_set(int_overflow, kern_code, handle4, interruptGate32, dpl_kern);
    idt_set(int_out_of_bound, kern_code, handle5, interruptGate32, dpl_kern);
    idt_set(int_invalid_operation_code, kern_code, handle6, interruptGate32, dpl_kern);
    idt_set(int_no_coprocessor, kern_code, handle7, interruptGate32, dpl_kern);
    idt_set(int_double_fault, kern_code, handle8, interruptGate32, dpl_kern);
    idt_set(int_coprocessor_segment_overrun, kern_code, handle9, interruptGate32, dpl_kern);
    idt_set(int_invalid_tss, kern_code, handle10, interruptGate32, dpl_kern);
    idt_set(int_no_segment, kern_code, handle11, interruptGate32, dpl_kern);
    idt_set(int_stack_segment_fault, kern_code, handle12, interruptGate32, dpl_kern);
    idt_set(int_protection, kern_code, handle13, interruptGate32, dpl_kern);
    idt_set(int_page_fault, kern_code, handle14, interruptGate32, dpl_kern);
    idt_set(int_reserved_15, kern_code, handle15, interruptGate32, dpl_kern);
    idt_set(int_math_fault, kern_code, handle16, interruptGate32, dpl_kern);
    idt_set(int_alignment_check, kern_code, handle17, interruptGate32, dpl_kern);
    idt_set(int_machine_check, kern_code, handle18, interruptGate32, dpl_kern);
    idt_set(int_simd, kern_code, handle19, interruptGate32, dpl_kern);
    idt_set(int_reserved_20, kern_code, handle20, interruptGate32, dpl_kern);
    idt_set(int_reserved_21, kern_code, handle21, interruptGate32, dpl_kern);
    idt_set(int_reserved_22, kern_code, handle22, interruptGate32, dpl_kern);
    idt_set(int_reserved_23, kern_code, handle23, interruptGate32, dpl_kern);
    idt_set(int_reserved_24, kern_code, handle24, interruptGate32, dpl_kern);
    idt_set(int_reserved_25, kern_code, handle25, interruptGate32, dpl_kern);
    idt_set(int_reserved_26, kern_code, handle26, interruptGate32, dpl_kern);
    idt_set(int_reserved_27, kern_code, handle27, interruptGate32, dpl_kern);
    idt_set(int_reserved_28, kern_code, handle28, interruptGate32, dpl_kern);
    idt_set(int_reserved_29, kern_code, handle29, interruptGate32, dpl_kern);
    idt_set(int_reserved_30, kern_code, handle30, interruptGate32, dpl_kern);
    idt_set(int_reserved_31, kern_code, handle31, interruptGate32, dpl_kern);

    idt_set(PIC_IR_VECTOR(ir_keyboard), kern_code, int33_handler, interruptGate32, dpl_kern);

    idt_set(44, kern_code, int44_handler, interruptGate32, dpl_kern);
    idt_set(46, kern_code, int46_handler, interruptGate32, dpl_kern);	/* IDE 0 */

    pic_reset_mask(ir_keyboard);

    for (i = 0; i < 128; i++) {
	intr_table[i].attr = 0;
	intr_table[i].func = 0;
    }

    intr_table[PIC_IR_VECTOR(ir_keyboard)].attr = 0;
    on_interrupt = 0;
    delayed_dispatch = FALSE;
    return (E_OK);
}

/*************************************************************************
 * interrupt --- 外部割り込みの処理
 *
 * 引数：	intn	割り込み番号
 *
 * 返値：	なし
 *
 * 処理：	外部割り込みが発生したときの処理を行う。
 *
 */
static int mask;
void interrupt(W intn)
{
    on_interrupt++;
    delayed_dispatch = FALSE;

    switch (intn) {
    default:
	if (intr_table[intn].func) {
	    (intr_table[intn].func) ();
	} else {
	    /* error!! */
	    printk("unknown interrupt from %d\n", intn);
	}
	break;

    case PIC_IR_VECTOR(ir_pit):
	intr_interval();
	break;

    case PIC_IR_VECTOR(ir_keyboard):
	if (intr_table[PIC_IR_VECTOR(ir_keyboard)].func != 0) {
	    (intr_table[PIC_IR_VECTOR(ir_keyboard)].func) ();
	}
	break;
    }

    /* 割込みの禁止フラグの OFF */
    if (intn > 40) {	/* slave */
      mask = ~(1 << (intn - 40));
      asm("cli");
      asm("inb $0xA1, %al");
      asm("andb mask, %al");
      asm("outb %al, $0xA1");
    } else {		/* master */
      mask = ~(1 << (intn - 32));
      asm("cli");
      asm("inb $0x21, %al");
      asm("andb mask, %al");
      asm("outb %al, $0x21");
    }

    enter_critical();
    --on_interrupt;
    leave_critical();

    if (delayed_dispatch && (on_interrupt == 0)) {
	thread_switch();
    }
}

/*
 * def_int システムコールによって、割り込みハンドラを登録する。
 *
 * 登録するときには、直接 IDT の値は変更せず、intr_table[] に登録する。
 */
ER set_interrupt_entry(W intno, FP func, ATR attr)
{
    if (intr_table[intno].attr == -1) {
	return (E_OBJ);
    }

    printk("set_interrupt_entry = %d, func = 0x%x\n", intno, func);
    intr_table[intno].attr = attr;
    intr_table[intno].func = func;
    return (E_OK);
}

/*************************************************************************
 * page_fault
 *
 * 引数： 
 *
 * 返値：
 *
 * 処理：
 *
 */
void page_fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		UW ecx, UW eax, UW es, UW ds, UW no,
		UW err, UW eip, UW cs, W eflags)
{
    W result;
    UW addr;
    T_REGION *regp;

    ++on_interrupt;
    if (run_task->page_fault_handler) {
      addr = (UW)fault_get_addr();
      /* フォルトを起こしたアドレスがスタック領域にあればページを割り当てる */
      regp = &run_task->regions[STACK_REGION];
      if (regp->permission &&
	  (((UW) regp->start_addr <= addr) &&
	   (addr <= ((UW) regp->start_addr + regp->max_size)))) {
	result = region_map(run_task->tskid, (VP) addr, PAGE_SIZE, ACC_USER);
	if (result == E_OK) {
	  /* ページフォルト処理に成功した */
	  --on_interrupt;
	  tlb_flush();
	  return;
	}
      }

      /* ページフォルト時の処理ハンドラが指定してあった */
      /* ページフォルトハンドラの引数は以下のとおり

       * ページフォルトが発生したアドレス
       * 実行している EIP
       * ページフォルト処理の result code
       */
      result = (run_task->page_fault_handler) ((UW)fault_get_addr(), eip);
      if (result == E_OK) {
	/* ページフォルト処理に成功した */
	--on_interrupt;
	return;
      }
      else {
	printk("page_fault_handler cause error\n");
      }
    }

    printk("page fault=%p\n", fault_get_addr());
    fault_with_error(edi, esi, ebp, esp, ebx, edx,
		ecx, eax, es, ds, no, err, eip, cs, eflags);

    --on_interrupt;

    for (;;);
}

/*************************************************************************
 * protect_fault --- 
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
void protect_fault(UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
		   UW ecx, UW eax, UW es, UW ds, UW no,
		   UW err, UW eip, UW cs, UW eflags)
{
    W result; 

    ++on_interrupt;
    if (run_task->page_fault_handler) {
	/* ページフォルト時の処理ハンドラが指定してあった */
	/* ページフォルトハンドラの引数は以下のとおり

	 * ページフォルトが発生したアドレス
	 * 実行している EIP
	 * ページフォルト処理の result code
	 */
	result = (run_task->page_fault_handler) ((UW)fault_get_addr(), eip);
	if (result == E_OK) {
	    /* ページフォルト処理に成功した */
	    --on_interrupt;
	    return;
	}
	else {
	  printk("page_fault_handler cause error\n");
	}
    }

    fault_with_error(edi, esi, ebp, esp, ebx, edx,
		ecx, eax, es, ds, no, err, eip, cs, eflags);
}
