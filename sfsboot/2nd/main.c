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
#include "time.h"
#include "macros.h"
#include "dma.h"
#include "fd.h"
#include "file.h"
#include "vram.h"
#include "page.h"
#include "interrupt.h"
#include "idt.h"
#include "ide.h"
#include "sfs.h"
#include "asm.h"

#include "main.h"

#define BOOT_IDE 0x01
#define BOOT_FDD 0x02

extern int evaluate(char* line);
int read_single_module (int start_block, void *paddr, struct module_info *info);

static void banner(void);

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
  int bootable = 0;
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
  if (ide_init () == E_OK) {
    bootable |= BOOT_IDE;
  }

  set_int();

  if (!bootable) {
    evaluate("boot fd 0");
    return;
  }

  for (;;)
    {
      boot_printf ("boot> ");
      gets (line);
      if (strlen ((unsigned char*)line) > 0)
	{
	  evaluate (line);
	}
    }
}

/***************************************************************************
 *
 */
static void
banner (void)
{
  /* boot_printf ("BTRON/386 2nd BOOT Version %d.%d\n\n", MAJOR_VER, MINOR_VER);
     boot_printf ("RCS Version : %s\n", rcsid); */
  boot_printf ("START AS 32 BIT MODE.\n");
  boot_printf ("Welcome BTRON/386.\n");
}
