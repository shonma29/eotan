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
#include "sync.h"
#include "../../kernel/mpu/interrupt.h"
#include "../../include/mpu/io.h"

#define KEY_COM		0x64
#define KEY_DATA	0x60
#define KEY_STAT	0x64

#define INT_KEYBOARD	33

/* status bit mask */
#define I_BUF_FULL_BIT	0x02

/* keyboard command */
#define RESET_COMMAND	0xF0


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

void system_reset()
{
  while ((inb(KEY_STAT) & I_BUF_FULL_BIT) == I_BUF_FULL_BIT)
    inb(KEY_DATA);
  enter_critical();
  outb(KEY_COM, RESET_COMMAND);
  asm("hlt");
}
