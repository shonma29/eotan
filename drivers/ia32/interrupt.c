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

#include <string.h>
#include <mpu/io.h>
#include "core.h"
#include "../../kernel/arch/arch.h"
#include "interrupt.h"
#include "thread.h"
#include "func.h"
#include "sync.h"
#include "mpufunc.h"

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
W on_interrupt = 0;
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
    GATE_DESC cg;

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

    set_idt(1, KERNEL_CSEG, (int) int1_handler, INTERRUPT_DESC, 0);
    set_idt(2, KERNEL_CSEG, (int) int2_handler, INTERRUPT_DESC, 0);
    set_idt(3, KERNEL_CSEG, (int) int3_handler, INTERRUPT_DESC, 0);
    set_idt(4, KERNEL_CSEG, (int) int4_handler, INTERRUPT_DESC, 0);
    set_idt(5, KERNEL_CSEG, (int) int5_handler, INTERRUPT_DESC, 0);
    set_idt(INT_INVALID_OPCODE, KERNEL_CSEG, (W) int6_handler, FAULT_DESC, 0);
    set_idt(INT_DOUBLE_FAULT, KERNEL_CSEG, (W) int8_handler, FAULT_DESC, 0);
    set_idt(INT_TSS_FAULT, KERNEL_CSEG, (W) int10_handler, FAULT_DESC, 0);
    set_idt(INT_INVALID_SEG, KERNEL_CSEG, (W) int11_handler, FAULT_DESC, 0);
    set_idt(INT_STACK_SEG, KERNEL_CSEG, (W) int12_handler, FAULT_DESC, 0);
    set_idt(INT_PROTECTION, KERNEL_CSEG, (W) int13_handler, FAULT_DESC, 0);
    set_idt(INT_PAGE_FAULT, KERNEL_CSEG, (W) int14_handler, FAULT_DESC, 0);
#if 0
    /* 将来のインテルプロセッサのために予約されている */
    set_idt(15, KERNEL_CSEG, (int) int15_handler, INTERRUPT_DESC, 0);
#endif
    set_idt(16, KERNEL_CSEG, (int) int16_handler, FAULT_DESC, 0);
    set_idt(17, KERNEL_CSEG, (int) int16_handler, FAULT_DESC, 0);
#if 0
    set_idt(18, KERNEL_CSEG, (int)int18_handler, INTERRUPT_DESC, 0);
#endif

    set_idt(35, KERNEL_CSEG, (int) int35_handler, INTERRUPT_DESC, 0);
    set_idt(37, KERNEL_CSEG, (int) int37_handler, INTERRUPT_DESC, 0);
    set_idt(INT_FD, KERNEL_CSEG, (int) int38_handler, INTERRUPT_DESC, 0);

    set_idt(44, KERNEL_CSEG, (int)int44_handler, INTERRUPT_DESC, 0);
    set_idt(46, KERNEL_CSEG, (int) int46_handler, INTERRUPT_DESC, 0);	/* IDE 0 */

#ifdef notdef
    set_idt(47, KERNEL_CSEG, (int) int47_handler, INTERRUPT_DESC, 0);
#endif

    /* システムコール用割込みルーチン */
    memset((VP)&cg, 0, sizeof(GATE_DESC));
    cg.type = TYPE_GATE;
    cg.dpl = USER_DPL;
    cg.present = 1;
    cg.selector = KERNEL_CSEG;
    SET_OFFSET_GATE(cg, (W) syscall_handler);
    set_gdt(SVC_GATE/sizeof(GEN_DESC), (GEN_DESC *) &cg);

    reset_intr_mask(3);
    reset_intr_mask(9);
#if 0
    reset_intr_mask(5);
    reset_intr_mask(6);
    reset_intr_mask(12);
#endif

    for (i = 0; i < 128; i++) {
	intr_table[i].attr = 0;
	intr_table[i].func = 0;
    }

    intr_table[INT_KEYBOARD].attr = 0;
    intr_table[INT_FD].attr = 0;
#ifdef notdef
    intr_table[INT_HD].attr = 0;
#endif
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

#ifdef notdef
    if ((intn != INT_TIMER) && (intn != INT_KEYBOARD))	/* */
	printk("Interrupt!! %d\n", intn);	/* */
#endif

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
#if 0
	    printk("%d[%d]", intn, run_task->tskid);
#endif
	    (intr_table[INT_KEYBOARD].func) ();
	}
	break;

    case INT_FD:
	if (intr_table[INT_FD].func != 0) {
	    (intr_table[INT_FD].func) ();
	}
	break;

#ifdef notdef
    case INT_HD:
	printk("interrupt: int HD\n");	/* */
	if (intr_table[INT_HD].func != 0) {
	    (intr_table[INT_HD].func) ();
	}
	break;
#endif
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
 * trap
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
void trap(W intn, W syscallno, VP arg)
{
    W i;

#ifdef DEBUG
    printk("trap: %d, run_task->n_interrupt = %d\n", intn,
	   run_task->n_interrupt);
#endif				/* DEBUG */
    if (run_task->n_interrupt > 0) {
	for (i = 0; i < run_task->n_interrupt; i++) {
#ifdef DEBUG
	    printk("interrupt handler searching...: %d, (trap no. = %d)\n",
		   i, run_task->interrupt[i].intr_no);
#endif				/* DEBUG */
	    if (run_task->interrupt[i].intr_no == intn) {
#ifdef DEBUG
		printk("found: %d(0x%x)\n",
		       run_task->interrupt[i].intr_func,
		       run_task->interrupt[i].intr_func);
#endif				/* DEBUG */
		(run_task->interrupt[i].intr_func) (syscallno, arg);
		return;
	    }
	}
    }

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
		UW ecx, UW eax, UW es, UW ds,
		UW errcode, UW eip, UW cs, W eflags)
{
    W result;
    UW addr;
    T_REGION *regp;

    ++on_interrupt;
    if (run_task->page_fault_handler) {
      addr = get_cr2();
      /* フォルトを起こしたアドレスがスタック領域にあればページを割り当てる */
      regp = &run_task->regions[STACK_REGION];
      if (regp->permission &&
	  (((UW) regp->start_addr <= addr) &&
	   (addr <= ((UW) regp->start_addr + regp->max_size)))) {
#if 1 /*def DEBUG*/
	printk("STACK PAGE FAULT (try to allocate page %x).\n", addr);
	printk("current PID= %d (0x%x)\n", run_task->tskid, run_task->tskid);
#ifdef notdef
	printk("current stack segment = 0x%x\n", run_task->context.ss);
	printk("       esp = 0x%x (0x%x)\n", esp, run_task->context.esp);
#endif
	printk("error code = 0x%x\n", errcode);
	printk("       eip = 0x%x\n", eip);
	printk("        cs = 0x%x ds = 0x%x\n", cs, ds);
	printk("    eflags = 0x%x\n", eflags);
#endif
	result = region_map(run_task->tskid, (VP) addr, PAGE_SIZE, ACC_USER);
	if (result == E_OK) {
	  /* ページフォルト処理に成功した */
	  --on_interrupt;
	  return;
	}
      }

#if 1 /*def DEBUG*/
      printk("PAGE FAULT (memory fault).\n");
      printk("current PID= %d (0x%x)\n", run_task->tskid, run_task->tskid);
#if 0
      printk("ss0 = 0x%x/ss1 = 0x%x/ss2 = 0x%x\n",
	     run_task->context.ss0,
	     run_task->context.ss1, run_task->context.ss2);
      printk("esp0 = 0x%x/esp1 = 0x%x/esp2 = 0x%x\n",
	     run_task->context.esp0,
	     run_task->context.esp1, run_task->context.esp2);
#endif
      printk("fault addr = 0x%x\n", get_cr2());
      printk("current stack segment = 0x%x\n", run_task->mpu.context.ss);
      printk("       esp = 0x%x (0x%x)\n", esp, run_task->mpu.context.esp);
      printk("error code = 0x%x\n", errcode);
      printk("       eip = 0x%x\n", eip);
      printk("        cs = 0x%x ds = 0x%x\n", cs, ds);
      printk("    eflags = 0x%x\n", eflags);
#endif
	/* ページフォルト時の処理ハンドラが指定してあった */
	/* ページフォルトハンドラの引数は以下のとおり

	 * ページフォルトが発生したアドレス
	 * 実行している EIP
	 * ページフォルト処理の result code
	 */
	result = (run_task->page_fault_handler) (get_cr2(), eip);
	if (result == E_OK) {
	    /* ページフォルト処理に成功した */
	    --on_interrupt;
	    return;
	}
	else {
	  printk("[ITRON] page_fault_handler cause error\n");
	}
    }

#if 0
    on_interrupt = TRUE;
#endif
    printk("KERNEL PAGE FAULT (memory fault).\n");
    printk("current PID= %d (0x%x)\n", run_task->tskid, run_task->tskid);
    printk("context addr = %d (0x%x)\n", run_task, run_task);
    printk("ss0 = 0x%x/ss1 = 0x%x/ss2 = 0x%x\n",
	   run_task->mpu.context.ss0,
	   run_task->mpu.context.ss1, run_task->mpu.context.ss2);
    printk("esp0 = 0x%x/esp1 = 0x%x/esp2 = 0x%x\n",
	   run_task->mpu.context.esp0,
	   run_task->mpu.context.esp1, run_task->mpu.context.esp2);

    printk("current stack segment = 0x%x\n", run_task->mpu.context.ss);
    printk("stack pointer = 0x%x\n", run_task->mpu.context.esp);
    printk("initial stack pointer = 0x%x\n", run_task->initial_stack);
    printk("fault addr = 0x%x\n", get_cr2());
    printk("       esp = 0x%x (0x%x)\n", esp, run_task->mpu.context.esp);
    printk("       cr3 = 0x%x (0x%x)\n", get_cr3(), run_task->mpu.context.cr3);
    printk("error code = 0x%x\n", errcode);
    printk("       eip = 0x%x\n", eip);
    printk("        cs = 0x%x\n", cs);
    printk("    eflags = 0x%x\n", eflags);
#ifdef notdef
    printk("        ss = 0x%x\n", ss);
#endif

#ifdef notdef
    {
	int i;
	unsigned char *p;

	p = (unsigned char *) eip;
	for (i = 0; i < 100; i++) {
	    printk("0x%x ", p[i]);
	}
    }
#endif

    --on_interrupt;

    for (;;);
}

/*************************************************************************
 * fault ---
 *
 * 引数：
 *
 * 返値：
 *
 * 処理：
 *
 */
void fault(UW intn, UW edi, UW esi, UW ebp, UW esp, UW ebx, UW edx,
	   UW ecx, UW eax, UW es, UW ds,
	   UW errcode, UW eip, UW cs, UW eflags)
{
    if (intn == 1) {
	leave_critical();
	printk("\nFAULT: Interrupt Number is %d ", intn);
	printk("PID = %d eflags = 0x%x\n", run_task->tskid, eflags);
	return;
    }

    enter_critical();
    printk("\nFAULT: Interrupt Number is %d.\n", intn);
    printk("PID = %d\n", run_task->tskid);
    printk("run_task->state.state = %d\n", run_task->tskstat);
/*  printk ("tss selector      = %d\n", run_task->tss_selector); */
    printk("run_task->context.backlink = 0%x\n",
	   run_task->mpu.context.backlink);
    printk("run_task->context.cs  = 0x%x\n", run_task->mpu.context.cs);
    printk("run_task->context.ds  = 0x%x\n", run_task->mpu.context.ds);
    printk("run_task->context.ss  = 0x%x\n", run_task->mpu.context.ss);
    printk("run_task->context.eip = 0x%x\n", run_task->mpu.context.eip);
    printk("run_task->context.esp = 0x%x\n", run_task->mpu.context.esp);
    printk("context addr = %d (0x%x)\n", run_task, run_task);
    printk("ss0 = 0x%x/ss1 = 0x%x/ss2 = 0x%x\n",
	   run_task->mpu.context.ss0,
	   run_task->mpu.context.ss1, run_task->mpu.context.ss2);
    printk("esp0 = 0x%x/esp1 = 0x%x/esp2 = 0x%x\n",
	   run_task->mpu.context.esp0,
	   run_task->mpu.context.esp1, run_task->mpu.context.esp2);

    printk("current stack segment = 0x%x\n", run_task->mpu.context.ss);
    printk("stack pointer = 0x%x\n", run_task->mpu.context.esp);
    printk("initial stack pointer = 0x%x\n", run_task->initial_stack);
    printk("error code = 0x%x\n", errcode);
    printk("fault addr = 0x%x\n", get_cr2());
    printk("       eip = 0x%x\n", eip);
    printk("        cs = 0x%x\n", cs);
    printk("    eflags = 0x%x\n", eflags);
    printk("       esp = 0x%x\n", esp);
    printk("       cr3 = 0x%x\n", get_cr3());
#if 0
    printk("        ss = 0x%x\n", ss);
#endif
    leave_critical();
    for (;;);
}

/**************************************************************************
 *
 */
W wait_int(W * flag)
{
    while (*flag == FALSE);
    enter_critical();
    *flag = FALSE;
    leave_critical();
    return (*flag);
}

/*************************************************************************
 * set_idt --- IDT のエントリをセットする。
 *
 * 引数：	entry
 *		selector
 *		offset
 *		type
 *		dpl
 *
 * 返値：	なし
 *
 * 処理：	IDT テーブルのエントリをセットする。
 *
 */
void set_idt(UW entry, UW selector, UW offset, UW type, UW dpl)
{
    struct idt_t *table;
#if 0
    UW *tmp;
#endif

    table = (struct idt_t *) IDT_ADDR;
    SET_OFFSET_IDT(table[entry], offset);
    table[entry].p = 1;
    table[entry].selector = selector;
    table[entry].dpl = dpl;
    table[entry].type = type;
    table[entry].dt0 = 0;
    table[entry].zero = 0;
#ifdef IDT_DEBUG
    printk("set_idt: entry = %d, selector = %d, type = %d\n",
	   entry, selector, type);
    tmp = (unsigned int *) &table[entry];
    printk("idt[%d] = 0x%x, 0x%x\n", entry, tmp[0], tmp[1]);
#endif				/* IDT_DEBUG */
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
		   UW ecx, UW eax, UW es, UW ds,
		   UW errcode, UW eip, UW cs, UW eflags)
{
    W result; 

    ++on_interrupt;
    if (run_task->page_fault_handler) {
#if 1 /*def DEBUG*/
      printk("PROTECT/STACK FAULT (memory fault).\n");
      printk("current PID= %d (0x%x)\n", run_task->tskid, run_task->tskid);
#if 0
      printk("ss0 = 0x%x/ss1 = 0x%x/ss2 = 0x%x\n",
	     run_task->context.ss0,
	     run_task->context.ss1, run_task->context.ss2);
      printk("esp0 = 0x%x/esp1 = 0x%x/esp2 = 0x%x\n",
	     run_task->context.esp0,
	     run_task->context.esp1, run_task->context.esp2);
      printk("current stack segment = 0x%x\n", run_task->context.ss);
#endif
      printk("       esp = 0x%x (0x%x)\n", esp, run_task->mpu.context.esp);
      printk("error code = 0x%x\n", errcode);
      printk("       eip = 0x%x\n", eip);
      printk("        cs = 0x%x ds = 0x%x\n", cs, ds);
      printk("    eflags = 0x%x\n", eflags);
#endif
	/* ページフォルト時の処理ハンドラが指定してあった */
	/* ページフォルトハンドラの引数は以下のとおり

	 * ページフォルトが発生したアドレス
	 * 実行している EIP
	 * ページフォルト処理の result code
	 */
	result = (run_task->page_fault_handler) (get_cr2(), eip);
	if (result == E_OK) {
	    /* ページフォルト処理に成功した */
	    --on_interrupt;
	    return;
	}
	else {
	  printk("[ITRON] page_fault_handler cause error\n");
	}
    }

    printk("******* KERNEL PROTECT/STACK FAULT: interrupt 13/12. ******\n");
    printk("PID = %d\n", run_task->tskid);
    printk("EIP = 0x%x ESP = 0x%x\n", eip, esp);
    printk("ERROR CODE = 0x%x eflags = 0x%x\n", errcode, eflags);
    printk("cs = 0x%x\n", cs);
    printk("run_task->state.state = %d\n", run_task->tskstat);
#if 0
    printk ("tss selector      = %d, 0x%x\n",
	  run_task->tss_selector, run_task->tss_selector);
    printk("STACK TOP: 0x%x\n", &(run_task->stackptr[run_task->stksz]));
    printk("STACK BASE: 0x%x\n", run_task->stackptr);
    printk("run_task->context.backlink  = 0x%x\n",
	   run_task->context.backlink);
    printk("run_task->context.ldtr  = 0x%x\n", run_task->context.ldtr);
    printk("run_task->context.cr3  = 0x%x\n", run_task->context.cr3);
    printk("run_task->context.eflags  = 0x%x\n", run_task->context.eflags);
    printk("run_task->context.cs  = 0x%x\n", run_task->context.cs);
    printk("run_task->context.ds  = 0x%x\n", run_task->context.ds);
    printk("run_task->context.ss  = 0x%x\n", run_task->context.ss);
#ifdef notdef
    printk("run_task->context.es  = 0x%x\n", run_task->context.es);
    printk("run_task->context.fs  = 0x%x\n", run_task->context.fs);
    printk("run_task->context.gs  = 0x%x\n", run_task->context.gs);
#endif				/* notdef */
    printk("run_task->context.eip = 0x%x\n", run_task->context.eip);
    printk("run_task->context.esp = 0x%x\n", run_task->context.esp);
#endif
    for (;;);
}
