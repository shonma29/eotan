#include "boot.h"
#include "lib.h"
#include "types.h"
#include "errno.h"
#include "config.h"
#include "location.h"
#include "memory.h"
#include "console.h"
#include "macros.h"
#include "dma.h"
#include "fd.h"
#include "file.h"
#include "vram.h"
#include "page.h"
#include "interrupt.h"
#include "idt.h"
#include "sfs.h"
#include "asm.h"

#include "main.h"

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
  init_8259A ();
  init_idt ();

  init_console ();
  banner ();

  init_memory ();

  init_vm ();
  enable_page ();

  init_fd ();

  set_int();

  boot_btron(0);
  return;
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
