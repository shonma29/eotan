/***************************************************************************
 * cinterrupt.c
 * このコードは,B-Free Project の boot/2nd/cinterrupt.c をもとにしたものです。
 *                                                  最終変更 2000/3/5
 * 
 *   
 *
 */

#include "types.h"
#include "asm.h"
#include "interrupt.h"
#include "lib.h"

UWORD ticks;

/****************************************************************************
 * init_8259A ---
 */
void
init_8259A(void)
{
/* init master 8259A */
  outb(MASTER_8259A_COM, 0x11);
  outb(MASTER_8259A_DATA, 0x20);
  outb(MASTER_8259A_DATA, 0x04);
  outb(MASTER_8259A_DATA, 0x01);
  
/* init slave 8259A */
  outb(SLAVE_8259A_COM, 0x11);
  outb(SLAVE_8259A_DATA, 0x28);
  outb(SLAVE_8259A_DATA, 0x02);
  outb(SLAVE_8259A_DATA, 0x01);

/* set mask */
  outb(MASTER_8259A_DATA, 0xfb);	/* 1111 1011 */
  outb(SLAVE_8259A_DATA,  0xff);	/* 1111 1111 */
}  

void
reset_intr_mask(int intn)
{
  clear_int();
  if(intn < 8)
    outb(MASTER_8259A_DATA, inb (MASTER_8259A_DATA) & ~(1 << intn));
  else
    outb(SLAVE_8259A_DATA, inb (SLAVE_8259A_DATA) & ~(1 << (intn - 8)));
  set_int();
}    

/***************************************************************************
 * interrupt --- 外部割り込みの処理
 */
void
interrupt(int intn)
{
  switch(intn)
    {
    default:
      /* error!! */
      boot_printf("unknown interrupt from %d\n", intn);
      break;
    case 32:
      ++ticks;
      break;

    case INT_KEYBOARD:
      intr_keyboard();
      break;

    case INT_FD:
      intr_fd();
      break;

    case INT_IDE:
      intr_ide();
      break;
    case INT_IDE2:
      intr_ide();
      break;
    }
}

void
trap(int intn)
{
  boot_printf("trap %d\n", intn);
}

void
fault(int intn)
{
  boot_printf("fault %d\n", intn);
}

/**************************************************************************
 *
 */
int
wait_int (int *flag)
{
  set_int();
  while(*flag == FALSE);
  clear_int();
  *flag = FALSE;
  set_int();
  return (*flag);
}

