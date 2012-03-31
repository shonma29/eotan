/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/beep/beeplow.c,v 1.1 1999/02/17 04:23:56 monaka Exp $ */
static char rcs[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/beep/beeplow.c,v 1.1 1999/02/17 04:23:56 monaka Exp $";


/*
 *
 *
 */

#include "beep.h"
#include "beep_int.h"

UW	initilized = 0;


/* BEEP アダプタが存在しているかどうかをチェックする
 */
ER
probe (struct device *dev)
{
  /*** ここに ***/

  return (E_OK);
}


/* 
 * $Log: beeplow.c,v $
 * Revision 1.1  1999/02/17 04:23:56  monaka
 * First version.
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */
