/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/pcmcia/pcmcialow.c,v 1.4 2000/02/21 22:34:23 kishida0 Exp $ */
static char rcs[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/pcmcia/pcmcialow.c,v 1.4 2000/02/21 22:34:23 kishida0 Exp $";


/*
 *
 *
 */

#include "pcmcia.h"
#include "pcmcia_internal.h"

UW	initilized = 0;


/* PCMCIA アダプタが存在しているかどうかをチェックする
 */
ER
probe (struct device *dev)
{
  return (E_OK);
}


void wreg(UW ind, UB data)
{
  outb(PCICREG,ind);
  outb(PCICREG+1,data);
}
UB   rreg(UW ind, UB data)
{
  outb(PCICREG,ind);
  return inb(PCICREG+1,data);
}

/* 
 * $Log: pcmcialow.c,v $
 * Revision 1.4  2000/02/21 22:34:23  kishida0
 * minor fix
 *
 * Revision 1.3  1999/07/06 13:40:13  kishida0
 * for debug
 *
 * Revision 1.2  1999/05/13 16:24:01  kishida0
 * デバッグ用にコード追加、特に大きな変更ではない
 *
 * Revision 1.1  1999/04/21 17:56:28  kishida0
 * for debug
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */
