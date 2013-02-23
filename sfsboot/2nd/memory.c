#include "types.h"
#include "location.h"
#include "config.h"
#include "memory.h"
#include "asm.h"

#undef USE_BIOS_CALL

UWORD32	base_mem, ext_mem, real_mem;

/**************************************************************************
 * init_memory
 *
 *
 */
void
init_memory (void)
{
  int *p;

#ifdef USE_BIOS_CALL
  p = (int*)0x8ff8;          /* initで書き込まれている */

  boot_printf ("Extended Memory = %d k bytes\n",*p / 1024);

  ext_mem = *p;
#else
  for (p = (int *)0x100000; (int)p < 0xf000000; p = (int*)((int)p + 0x100000))
    {
      *p = 0;
      *p = 0xAA;
      asm("WBINVD"); /* 坂本＠愛知県さんによる cpu_flush ルーチンから */
      if (*p != 0xAA)
	break;
    }
  boot_printf ("Extended Memory = %d K bytes\n", ((int)p - 0x100000) / 1024);
  ext_mem = ((int)p - 0x100000);
#endif

  base_mem = BASE_MEM;
  real_mem = ext_mem + BASE_MEM;
  boot_printf ("Real Memory = %d K bytes\n", real_mem / 1024);
}

