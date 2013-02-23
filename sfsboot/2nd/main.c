#include "bootos.h"
#include "lib.h"
#include "types.h"
#include "memory.h"
#include "console.h"
#include "fd.h"
#include "page.h"
#include "interrupt.h"
#include "idt.h"
#include "asm.h"

static void banner(void);

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

  ena_int();

  boot_btron(0);
  return;
}

/***************************************************************************
 *
 */
static void
banner (void)
{
  boot_printf ("START AS 32 BIT MODE.\n");
  boot_printf ("Welcome BTRON/386.\n");
}
