#include "boot.h"
#include "lib.h"
#include "types.h"
#include "errno.h"
#include "config.h"
#include "location.h"
#include "memory.h"
#include "console.h"
#include "keyboard.h"
#include "date.h"
#include "macros.h"
#include "dma.h"
#include "fd.h"
#include "file.h"
#include "vram.h"
#include "page.h"
#include "a.out.h"		/* for gcc header */
#include "interrupt.h"
#include "idt.h"
#include "ide.h"
#include "sfs.h"
#include "asm.h"

#include "main.h"

extern int evaluate(char* line);
int read_single_module (int start_block, void *paddr, struct module_info *info);

struct file_system	root_fs;
struct file		root;
struct file		cwd;

/******************************************************************************
 *
 *
 */
void
_main ()
{
  char	line[MAX_LINE];
  extern int setidt ();
  extern UWORD ticks;

  ticks = 0;
  init_8259A ();
  init_idt ();

  /* set up timer */
  outb(TIMER_CONTROL, 0x36);	/* MODE 3 */
  outb(TIMER0_WRITE, (TIMER_FREQ/TICKS) & 0xff);
  outb(TIMER0_WRITE, ((TIMER_FREQ/TICKS) >> 8) & 0xff);

  init_console ();
  banner ();

  init_memory ();
  init_time();
  init_keyboard ();

  init_vm ();
  enable_page ();

  init_fd ();
  ide_init ();

  set_int();

  for (;;)
    {
      boot_printf ("boot> ");
      gets (line);
      if (strlen (line) > 0)
	{
	  evaluate (line);
	}
    }
}

/***************************************************************************
 *
 */
void
banner (void)
{
  /* boot_printf ("BTRON/386 2nd BOOT Version %d.%d\n\n", MAJOR_VER, MINOR_VER);
     boot_printf ("RCS Version : %s\n", rcsid); */
  boot_printf ("START AS 32 BIT MODE.\n");
  boot_printf ("Welcome BTRON/386.\n");
}

/**************************************************************************
 * panic.
 */
void
panic (char *s)
{
  boot_printf ("panic: %s\n", s);
  for (;;)
    ;
}

int
cat (char *buf, int size)
{
  int	i;

  for (i = 0; i < size; i += 2)
    {
      boot_printf ("%c", &buf[i]);
    }

  return E_OK;
}






