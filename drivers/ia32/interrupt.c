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
#include "../../kernel/arch/arch.h"
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
    /* 8259 の初期化 */
    /* init master 8259A */
    outb(MASTER_8259A_COM, 0x11);	/* ICW1 */
    outb(MASTER_8259A_DATA, 0x20);	/* ICW2 */
    outb(MASTER_8259A_DATA, 0x04);	/* ICW3 */
    outb(MASTER_8259A_DATA, 0x01);	/* ICW4 */

    /* init slave 8259A */
    outb(SLAVE_8259A_COM, 0x11);	/* ICW1 */
    outb(SLAVE_8259A_DATA, 0x28);	/* ICW2 */
    outb(SLAVE_8259A_DATA, 0x02);	/* ICW3 */
    outb(SLAVE_8259A_DATA, 0x01);	/* ICW4 */

/* set mask */
    outb(MASTER_8259A_DATA, 0xfb);	/* 1111 1011 */
    outb(SLAVE_8259A_DATA, 0xff);	/* 1111 1111 */

    idt_set(0, kern_code, handle0, interruptGate32, dpl_kern);
    idt_set(1, kern_code, handle1, interruptGate32, dpl_kern);
    idt_set(2, kern_code, handle2, interruptGate32, dpl_kern);
    idt_set(3, kern_code, handle3, interruptGate32, dpl_kern);
    idt_set(4, kern_code, handle4, interruptGate32, dpl_kern);
    idt_set(5, kern_code, handle5, interruptGate32, dpl_kern);
    idt_set(INT_INVALID_OPCODE, kern_code, handle6, interruptGate32, dpl_kern);
    idt_set(7, kern_code, handle7, interruptGate32, dpl_kern);
    idt_set(INT_DOUBLE_FAULT, kern_code, handle8, interruptGate32, dpl_kern);
    idt_set(9, kern_code, handle9, interruptGate32, dpl_kern);
    idt_set(INT_TSS_FAULT, kern_code, handle10, interruptGate32, dpl_kern);
    idt_set(INT_INVALID_SEG, kern_code, handle11, interruptGate32, dpl_kern);
    idt_set(INT_STACK_SEG, kern_code, handle12, interruptGate32, dpl_kern);
    idt_set(INT_PROTECTION, kern_code, handle13, interruptGate32, dpl_kern);
    idt_set(INT_PAGE_FAULT, kern_code, handle14, interruptGate32, dpl_kern);
    idt_set(15, kern_code, handle15, interruptGate32, dpl_kern);
    idt_set(16, kern_code, handle16, interruptGate32, dpl_kern);
    idt_set(17, kern_code, handle17, interruptGate32, dpl_kern);
    idt_set(18, kern_code, handle18, interruptGate32, dpl_kern);
    idt_set(19, kern_code, handle19, interruptGate32, dpl_kern);
    idt_set(20, kern_code, handle20, interruptGate32, dpl_kern);
    idt_set(21, kern_code, handle21, interruptGate32, dpl_kern);
    idt_set(22, kern_code, handle22, interruptGate32, dpl_kern);
    idt_set(23, kern_code, handle23, interruptGate32, dpl_kern);
    idt_set(24, kern_code, handle24, interruptGate32, dpl_kern);
    idt_set(25, kern_code, handle25, interruptGate32, dpl_kern);
    idt_set(26, kern_code, handle26, interruptGate32, dpl_kern);
    idt_set(27, kern_code, handle27, interruptGate32, dpl_kern);
    idt_set(28, kern_code, handle28, interruptGate32, dpl_kern);
    idt_set(29, kern_code, handle29, interruptGate32, dpl_kern);
    idt_set(30, kern_code, handle30, interruptGate32, dpl_kern);
    idt_set(31, kern_code, handle31, interruptGate32, dpl_kern);

    idt_set(INT_KEYBOARD, kern_code, int33_handler, interruptGate32, dpl_kern);
    idt_set(INT_FD, kern_code, int38_handler, interruptGate32, dpl_kern);

    idt_set(44, kern_code, int44_handler, interruptGate32, dpl_kern);
    idt_set(46, kern_code, int46_handler, interruptGate32, dpl_kern);	/* IDE 0 */

    reset_intr_mask(1);
    reset_intr_mask(3);
    reset_intr_mask(9);

    for (i = 0; i < 128; i++) {
	intr_table[i].attr = 0;
	intr_table[i].func = 0;
    }

    intr_table[INT_KEYBOARD].attr = 0;
    intr_table[INT_FD].attr = 0;
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

    case INT_TIMER:
	intr_interval();
	break;

    case INT_KEYBOARD:
	if (intr_table[INT_KEYBOARD].func != 0) {
	    (intr_table[INT_KEYBOARD].func) ();
	}
	break;

    case INT_FD:
	if (intr_table[INT_FD].func != 0) {
	    (intr_table[INT_FD].func) ();
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
	printk("[KERN] page_fault_handler cause error\n");
      }
    }

    idt_abort_with_error(edi, esi, ebp, esp, ebx, edx,
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
	  printk("[KERN] page_fault_handler cause error\n");
	}
    }

    idt_abort_with_error(edi, esi, ebp, esp, ebx, edx,
		ecx, eax, es, ds, no, err, eip, cs, eflags);
}
