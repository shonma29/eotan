/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/ITRON/kernlib/sys_lowlib.c,v 1.1 1999/04/18 17:48:34 monaka Exp $ */

static unsigned char rcsid[] = "$Id: sys_lowlib.c,v 1.1 1999/04/18 17:48:34 monaka Exp $";


#include "../../ITRON/h/types.h"
#include "../../ITRON/h/itron.h"
#include "../../ITRON/h/syscall.h"
#include "../../ITRON/h/errno.h"
#include "../../ITRON/h/lowlib.h"


/* lowlib ������Υ������� attach ����
 */
ER
lod_low (ID task, B *name)
{
  ER erResult;
  struct a
    {
      ID task;
      B *name;
    } args;

  args.task = task;
  args.name = name;

  erResult = call_syscall (SYS_VSYS_MSC, 3, &args);

  return (erResult);
}


/* lowlib �ξ�������
 */
ER
sts_low (B *name, struct lowlib_info *infop, W *nlowlib)
{
  ER erResult;
  struct a
    {
      B				*name;
      struct lowlib_info	*infop;
      W				*nlowlib; /* lowlib ����Ͽ�� (name == */
					  /* NULL �ΤȤ�) 	    */
    } args;

  args.name = name;
  args.infop = infop;
  args.nlowlib = nlowlib;
  
  erResult = call_syscall (SYS_VSYS_MSC, 5, &args);

  return (erResult);
}

