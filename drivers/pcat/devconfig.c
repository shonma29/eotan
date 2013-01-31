/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* 
 * ITRON 内部のデバイスドライバの定義
 *
 */

#include <itron/types.h>
#include "config.h"
#include "arch/archfunc.h"

static ER (*devices[])(void) = {
  init_keyboard,
  0
};


void device_initialize(void)
{
    W i;

#ifdef DEBUG
    printk("device_initialize: start.\n");
#endif
    for (i = 0; devices[i] != NULL; i++) {
#ifdef DEBUG
	printk("Init device: 0x%x call.\n", (*devices[i]));
#endif
	(*devices[i]) ();
    }
#ifdef DEBUG
    printk("device_initialize: end.\n");
#endif
}

