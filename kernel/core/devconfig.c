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

#include "types.h"
#include "config.h"

extern ER	init_keyboard (void);

ER (*devices[MAX_DEVICE])(void) = {
  init_keyboard,
  0,
};

W ndevice = (sizeof (devices) / sizeof (devices[0]));
