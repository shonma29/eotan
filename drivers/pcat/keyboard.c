/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002-2003, Tomohide Naniwa

*/
/* keyboard.c
 *
 *
 */

#include "core.h"
#include "task.h"
#include "func.h"
#include "misc.h"
#include "../../kernel/mpu/interrupt.h"
#include "../../include/mpu/io.h"
#include "../../include/keycode.h"

#define KEY_COM		0x64
#define KEY_DATA	0x60
#define KEY_STAT	0x64

#define INT_KEYBOARD	33

/* status bit mask */
#define I_BUF_FULL_BIT	0x02

/* keyboard command */
#define AUX_ITF_DISABLE	0xA7
#define KBD_ITF_DISABLE	0xAD
#define OUTPORT_WRITE	0xD1
#define RESET_COMMAND	0xF0

#define ISBREAK(ch)     (ch & 0x80)

struct key_entry {
    struct key_entry *next;
    UW ch;
};


struct keyboard {
    UW shiftkey;		/* シフトキーの状態を表す               */
    UW total;			/* 現在キューイングしているエントリ数   */
    struct key_entry *first;	/* キューイングリストの先頭エントリ     */
    struct key_entry *last;	/* キューイングリストの最終エントリ     */
};


static struct keyboard input_buffer;
static struct key_entry *freeentry;


static struct key_entry *alloc_key(UW code)
{
    struct key_entry *p;

    if (freeentry == NULL) {
	return (NULL);
    }
    p = freeentry;
    freeentry = freeentry->next;
    p->ch = code;
    p->next = NULL;
    return (p);
}


/****************************************************************************
 * init_keyboard --- 
 *
 */
ER init_keyboard(void)
{
    W status;

    set_idt(INT_KEYBOARD, 0x08, (int) int33_handler, INTERRUPT_DESC, 0);
    reset_intr_mask(1);
    status = inb(KEY_STAT);
    printk("keyboard status: 0x%x\n", status);

    return (E_OK);
}

/*
 * キーボード割り込みによって起動される関数。
 *
 * キーボードデバイスからキーイベントを読み取り、入力バッファ (input_buffer) 
 * に追加する。
 */
W intr_kbd(void)
{
    W ch;
    struct key_entry *key;
    ER error;
    W i;
    static W mode = 0;


    ch = inb(KEY_DATA);
    if (ch == 0xE0) {
	for (i = 0; i < 100; i++) {
	    if ((inb(KEY_STAT) & I_BUF_FULL_BIT) == 0)
		break;
	}
	ch = inb(KEY_DATA);	/* get scan code */
	for (i = 0; i < 100; i++) {
	    if ((inb(KEY_STAT) & I_BUF_FULL_BIT) == 0)
		break;
	}
	if (i == 100) {
	    printk("keyboard wait overrun\n");
	}
#ifdef KEYBOARD_DEBUG
	printk("keycode = 0x%x\n", ch);
#endif
	ch = ch | ENCAP;
	if ((ch & 0xff) == 0x53) {	/* delete key */
	    if (mode & (SHIFT | CTRL)) {
		printk("reset\n");
	    }
	}
    } else if ((ch == 0x2a) || (ch == 0x36)) {
	mode |= SHIFT;
	return E_OK;
    } else if (ch == 0x1d) {
	mode |= CTRL;
	return E_OK;
    } else if (ch == 0x38) {
	mode |= ALT;
	return E_OK;
    } else if (ISBREAK(ch)) {
	ch = ch & 0x7f;

	/* シフトキー等の場合、状態を変更する */
	if ((ch == 0x2a) || (ch == 0x36)) {
	    mode &= ~SHIFT;
	} else if (ch == 0x1d) {
	    mode &= ~CTRL;
	} else if (ch == 0x38) {
	    mode &= ~ALT;
	}
	return E_OK;
    }

    dis_int();
    key = alloc_key(ch);
    if (key == NULL) {		/* キーがない */
	ena_int();
	return E_OK;
    }

    key->ch = ch | mode;
    if (input_buffer.last) {
	input_buffer.last->next = key;
	input_buffer.last = key;
    } else {
	input_buffer.first = input_buffer.last = key;
    }
    ena_int();

    if ((error = wup_tsk(ITRON_KEYBOARD)) != E_OK) {
	if (error != E_OBJ) {
	    printk("errno = %d\n", error);
	    falldown("intr_kbd: error on wup_tsk.\n");
	}
    }
    return E_OK;
}


void system_reset()
{
  while ((inb(KEY_STAT) & I_BUF_FULL_BIT) == I_BUF_FULL_BIT)
    inb(KEY_DATA);
  dis_int();
  outb(KEY_COM, RESET_COMMAND);
  asm("hlt");
}
