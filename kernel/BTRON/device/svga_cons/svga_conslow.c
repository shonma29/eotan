/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* @(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/svga_cons/svga_conslow.c,v 1.1 1999/04/11 14:41:33 kishida0 Exp $ */
static char rcs[] = "@(#)$Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/BTRON/device/svga_cons/svga_conslow.c,v 1.1 1999/04/11 14:41:33 kishida0 Exp $";


/*
 *
 *
 */

#include "svga_cons.h"
#include "svga_cons_internal.h"

UW	initilized = 0;


/* SVGA_CONS �����ץ���¸�ߤ��Ƥ��뤫�ɤ���������å�����
 */
ER
probe (struct device *dev)
{
  /*** ������ ***/

  return (E_OK);
}


/* 
 * $Log: svga_conslow.c,v $
 * Revision 1.1  1999/04/11 14:41:33  kishida0
 * for debug function
 *
 * Revision 1.1  1998/12/19 07:50:25  monaka
 * Pre release version.
 *
 */
