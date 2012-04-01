/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/beep/beep_internal.h,v 1.2 1999/04/18 18:43:16 monaka Exp $ */

/* 
 * $Log: beep_internal.h,v $
 * Revision 1.2  1999/04/18 18:43:16  monaka
 * Fix for moving Port-manager and libkernel.a  to ITRON. I guess it is reasonable. At least they should not be in BTRON/.
 *
 * Revision 1.1  1999/02/17 04:23:56  monaka
 * First version.
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */


#ifndef	__BEEP_INTERNAL_H__
#define	__BEEP_INTERNAL_H__	1


#include "h/itron.h"
#include "h/errno.h"
#include "../console/console.h"
#include "servers/port-manager.h"


/*** ここにデバイスの定義をかいてください ***/


/* デバイス構造体 
 * BEEP デバイスは、各々この構造体で管理している
 */
struct device
{
  /*** ここに ***/
};



#include "global.h"
#include "funcs.h"


#endif /* __BEEP_INTERNAL_H__ */
