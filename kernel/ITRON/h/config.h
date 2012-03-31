/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2001-2002, Tomohide Naniwa

*/
/* 
  This file is part of BTRON/386

  $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/h/config.h,v 1.9 2000/07/10 14:22:05 kishida0 Exp $

  

*/

#ifndef _ITRON_CONFIG_H_
#define _ITRON_CONFIG_H_	1

#define CALL_HANDLER_IN_TASK	1 /* �����ޡ��Υϥ�ɥ顼�μ¹Ԥ� task 1 �ǹԤ� */

#include "../i386/i386.h"

#define GDT_ADDR	(0x80001000UL)
#define IDT_ADDR	(0x80002000UL)
#define MAX_GDT		(255)	/* GDT �κ���Υ���ǥå��� */
#define MAX_IDT		(255)	/* IDT �κ���Υ���ǥå��� */

#define TASK_DESC_MIN	TSS_BASE
#define KERNEL_CSEG	(0x0008)	/* �����ͥ�Υ����ɥ������� */
#define KERNEL_DSEG	(0x0010)	/* �����ͥ�Υǡ����������� */
#define USER_CSEG	(0x0018)	/* �桼���Υ����ɥ������� */
#define USER_DSEG	(0x0020)	/* �桼���Υǡ����������� */
#define USER_SSEG	(0x0028)	/* �桼���Υ����å��������� */
#define ITRON_GATE	(0x0030)	/* ITRON �����ƥॳ�����ѥ����� */
#define POSIX_GATE	(0x0038)	/* POSIX �����ƥॳ�����ѥ����� */
#define BTRON_GATE	(0x0040)	/* BTRON �����ƥॳ�����ѥ����� */

#define PAGE_DIR_ADDR	(0x3000)	/* ���ۥڡ����ǥ��쥯�ȥ�ơ��֥� */
#define PAGE_ENTRY_ADDR	(0x4000)	/* ���ۥڡ����ơ��֥� */

/* �����������ط� */
#define MAX_TSKID	(128)	/* ����Υ����� ID ��   */
#define MIN_TSKID	(1)	/* �Ǿ��Υ����� ID ��   */
#define NTASK		(MAX_TSKID - MIN_TSKID + 1)	/* ��������     */

#define MIN_PRIORITY	(0)	/* �Ǿ��Υץ饤����ƥ� */
#define MAX_PRIORITY	(31)	/* ����Υץ饤����ƥ� */

#define MIN_USERTASKID	(21)
#define MAX_USERTASKID	MAX_TSKID

#define MAX_SUSPEND_NEST	(10)	/* �����ڥ�ɤΥͥ��ȤΥ����С��ե� */

#define KERNEL_DPL	0
#define USER_DPL	3

#define KERNEL_LEVEL	(2)
#define USER_LEVEL	(16)
#define MID_LEVEL	(10)

#define KERNEL_STACK_SIZE	(PAGE_SIZE * 10)
#define POSIX_STACK_SIZE	(PAGE_SIZE * 10)

#define CLOCK		10		/* clock ���� 10 ms */
#define TICKS		(1000/CLOCK)	/* 1 �ô֤� tick �� */
#define QUANTUM		(40/CLOCK)	/* 40 ms */
#define HALT_WHEN_IDLE	1	/* IDLE ���� CPU �� Halt ����ߤ��� */

/* ���ޥե������ط� */
#define MIN_SEMAID		(1)
#define MAX_SEMAID		(256)
#define NSEMAPHORE		(MAX_SEMAID - MIN_SEMAID + 1)
#define MIN_USERSEMAID		(10)
#define MAX_USERSEMAID		MAX_SEMAID


/* ���٥�ȥե饰�����ط� */
#define MIN_EVENTFLAG		(1)
#define MAX_EVENTFLAG		(256)
#define NEVENTFLAG		(MAX_SEMAID - MIN_SEMAID + 1)
#define MIN_USEREVENTID		(10)
#define MAX_USEREVENTID		MAX_EVENTFLAG


/* ��å������Хåե������ط� */
#define MIN_MSGBUF		(1)
#define MAX_MSGBUF		(256)
#define NMSGBUF			(MAX_MSGBUF - MIN_MSGBUF + 1)
#define MAX_MSGENTRY		(1000)
/*
 * ��ͳ�˻Ȥ����å������Хåե��κǾ��� ID �Ⱥ���� ID
 */
#define MIN_USERMBFID		(100)
#define MAX_USERMBFID		MAX_MSGBUF


/* ��������ط� */
#define MAX_MEMPOOL	(100)
#ifndef PAGE_SIZE
#define PAGE_SIZE	I386_PAGESIZE
#endif
#define PAGE_SHIFT	(12)
#define DIR_SHIFT	(22)
#define PAGE_MASK	(0x003FF000L)
#define DIR_MASK	(0xFFC00000UL)
#define OFFSET_MASK	(0x00000FFFL)
#define	MIN_KERNEL	(0x80000000UL)
#define MAX_KERNEL	(0xFFFFFFFFUL)
#define KERNEL_SIZE	(MAX_KERNEL - MIN_KERNEL)

#define MEM_SIZE		(0x100000)	/* ��ĥ����� 1M �Х��� */
#define MAX_FREEMEM_ENTRY	(1000)	/* �ե꡼�����������뤿��Υơ��֥� */
#define MAX_MEMORY_POOL		(100)
#define MIN_MEMORY_SIZE		(16 * 1024 * 1024) /* ����ϺǾ� 16 M */
#define MAX_MEMORY_SIZE		(128 * 1024 * 1024) /* ����Ϻ��� 128 M */
#define MEMORY_POOL_BASE	(0x80010000UL)	/* �����ƥ����Ȥ��ƴ��������ΰ�Υ١��� */

#define SYSTEM_MEMORY_POOL	(0)	/* �����ƥ����ס���� ID */

#define MAX_REGION		(5)	/* �ƥ�������¸�ߤǤ��� REGION �ο� */
#define KERNEL_REGION		0
#define TEXT_REGION		1
#define DATA_REGION		2
#define HEAP_REGION		3
#define STACK_REGION		4

/* �ޥ͡����㡼�������Υҡ����ΰ�ο侩�� */
#define VADDR_HEAP	(0x10000000L)
#define STD_HEAP_SIZE	(0x3FFFFFFFL)

#define STD_STACK_SIZE	 (0x3FF00000L)
#define VADDR_STACK_TAIL (0x7FF00000L)
#define VADDR_STACK_HEAD (VADDR_STACK_TAIL - STD_STACK_SIZE)

/* IO �����ط� */
#define MAX_DEVICE_NAME		(16)	/* �ǥХ���̾�κ���Ĺ (16 �Х���) */
#define MIN_DEVID		(0)
#define MAX_DEVID		(19)
#define MAX_DEVICE		(MAX_DEVID - MIN_DEVID + 1)	/* �ǥХ����κ���� */


/* ���󥿡��Х륿���� */
#define MAX_TIMER	(500)


/* �����ƥॿ�����Υꥹ�� */
#define NSYSTEM_TASK	(20)	/* �����ƥॿ�����ο� */

#define ITRON_IDLE	(1)
#define ITRON_RS232C	(2)	/* ITRON �� RS232C �ɥ饤�Х����� */
#define ITRON_KEYBOARD	(3)
#define ITRON_CONSOLE	(4)	/* ���󥽡���ǥХ����ɥ饤�� */
#define ITRON_DEBUG	(5)	/* �ǥХå��ѥץ���: ���ޥ�ɥ��󥿥ץ꥿�� */

/*
 * �ݡ��ȥޥ͡����㤬�׵�����Ĥ��Ѥ˻��ꤹ���å������Хåե��� ID 
 */
#define PORT_MANAGER_PORT	(11)


/* �����ƥॻ�ޥե��Υꥹ�� */
#define ITRON_KEYBOARD_SEM	(3)	/* �����ܡ��������ѤΥ��ޥե�    */

/* ���ִ��� */
#define MAX_CYCLIC		(50)	/* ������ư�ϥ�ɥ�κ���� */
#define MAX_ALARM		(50)	/* ���顼��ϥ�ɥ�κ���� */

#define FDC_ALARM		(0)

/* �����ƥ��å������Хåե��Υꥹ�� */
#define ITRON_DEBUG_MBF		(1)	/* �ǥХå��ѥץ���: ���ޥ�ɥ��󥿥ץ꥿�ǻ��� */
#define ITRON_RS232C_MBF	(2)	/* ITRON �� RS232C �ɥ饤�Х������ǻ��� */
#define ITRON_KEYBOARD_MBF	(3)	/* �����ܡ��������ѤΥ�å������Хåե� */


extern W ndevice;
extern ER(*devices[]) (void);


#endif				/* _ITRON_CONFIG_H_ */
