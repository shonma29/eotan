/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/*
 *
 *
 */

static char rcsid[] = "$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/common/system.c,v 1.7 1999/04/13 04:49:06 monaka Exp $";

#include "core.h"
#include "boot.h"
#include "api.h"
#include "func.h"

ER
sys_vsys_inf (VP argp)
{
  struct 
    {
      W func;
      W sub;
      VP buf;
    } *args = argp;
  W		  *rootfs;
  struct boot_header	*info;
  

  switch (args->func)
    {
      
    case ROOTFS_INFO:
      rootfs = (W *)args->buf;
      info = (struct boot_header *)(MODULE_TABLE | 0x80000000);
      *rootfs = info->machine.rootfs;
      printk ("vsys_inf: rootfs_info: rootfs = 0x%x\n", info->machine.rootfs);	/* */
      return (E_OK);

    default:
      return (E_PAR);
    }

}
