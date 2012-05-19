/**************************************************************************
* idt.c
* このコードは,B-Free Project の boot/2nd/idt.c をもとにしたものです。 
* 
*                                                最終変更 2000/3/5  
*/

#include "types.h"
#include "errno.h"
#include "../../kernel/core/config.h"
#include "idt.h"
#include "interrupt.h"

/*********************************************************************
 * init_idt
 */
void
init_idt (void)
{
  int	i;

  for (i = 0; i < 0x20; i++)
    {
      set_idt (i, 0x08, (int)ignore_handler, TRAP_DESC, 0);
    }

  set_idt (0x20, 0x08, (int)int32_handler, INTERRUPT_DESC, 0); /*ハードウェア割り込み */
  set_idt (0x21, 0x08, (int)int33_handler, INTERRUPT_DESC, 0);
  set_idt (0x23, 0x08, (int)int35_handler, INTERRUPT_DESC, 0);
  set_idt (0x24, 0x08, (int)int36_handler, INTERRUPT_DESC, 0);
  set_idt (0x25, 0x08, (int)int37_handler, INTERRUPT_DESC, 0);
  set_idt (0x26, 0x08, (int)int38_handler, INTERRUPT_DESC, 0);
  set_idt (0x27, 0x08, (int)int39_handler, INTERRUPT_DESC, 0);
  set_idt (0x28, 0x08, (int)int40_handler, INTERRUPT_DESC, 0); /* Slave 8259A */
  set_idt (0x29, 0x08, (int)int41_handler, INTERRUPT_DESC, 0);
  set_idt (0x2a, 0x08, (int)int42_handler, INTERRUPT_DESC, 0);
  set_idt (0x2b, 0x08, (int)int43_handler, INTERRUPT_DESC, 0);
  set_idt (0x2c, 0x08, (int)int44_handler, INTERRUPT_DESC, 0);
  set_idt (0x2d, 0x08, (int)int45_handler, INTERRUPT_DESC, 0);
  set_idt (0x2e, 0x08, (int)int46_handler, INTERRUPT_DESC, 0);
  set_idt (0x2f, 0x08, (int)int47_handler, INTERRUPT_DESC, 0);

  for (i = 0x37; i < MAX_IDT; i++)
    {
      set_idt (i, 0x08, (int)ignore_handler, TRAP_DESC, 0);
    }
  

}

/*********************************************************************
 * set_idt
 */
int
set_idt (int entry, int selector, int offset, int type, int dpl)
{
  struct idt_t	*table;
  
  table = (struct idt_t *)IDT_TABLE_ADDR;
  SET_OFFSET_IDT (table[entry], offset);
  table[entry].p = 1;
  table[entry].selector = selector;
  table[entry].dpl = dpl;
  table[entry].type = type;
  table[entry].dt0 = 0;
  table[entry].zero = 0;

  return E_OK;
}


/*********************************************************************
 *	get_idt
 */
struct idt_t *
get_idt (int entry)
{
  struct idt_t *table;

  table = (struct idt_t *)IDT_TABLE_ADDR;
  return (&table[entry]);
}
