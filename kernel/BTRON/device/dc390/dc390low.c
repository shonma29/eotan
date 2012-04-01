/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/dc390/dc390low.c,v 1.2 1999/06/20 14:46:43 kishida0 Exp $ */
static char rcs[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/dc390/dc390low.c,v 1.2 1999/06/20 14:46:43 kishida0 Exp $";


/*
 *
 *
 */

#include "dc390.h"
#include "dc390_internal.h"

UW	initilized = 0;


/* DC390 アダプタが存在しているかどうかをチェックする
 */
ER
probe (struct device *dev)
{
  /*** ここに ***/

  return (E_OK);
}

void 
GetPCI-ConigurationSpaceHeader(CPI-INF *pci)
{
  int i;
  U32B *p;
  p=(U32B)pci;

  for(i=0 ; i<=0x4c ; i=i+4){
    *p = in_dw(i);
    p++
  }

}


/* 
 * $Log: dc390low.c,v $
 * Revision 1.2  1999/06/20 14:46:43  kishida0
 * write defines for debug.
 *
 * Revision 1.1  1999/04/26 18:33:32  kishida0
 * add for debug
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */
