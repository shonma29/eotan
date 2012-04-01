/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

(C) 2002, Tomohide Naniwa

*/

/* $Id: start.c,v 1.2 2000/02/07 18:09:21 kishida0 Exp $ */
static char rcsid[] = "$Id: start.c,v 1.2 2000/02/07 18:09:21 kishida0 Exp $";

/*
 * $Log: start.c,v $
 * Revision 1.2  2000/02/07 18:09:21  kishida0
 * add proto type define
 *
 * Revision 1.1  1996/07/25 16:02:00  night
 * IBM PC 版用への最初の登録
 *
 * Revision 1.1  1996/03/04  19:52:21  night
 * 最初の登録
 *
 *
 */

#include "init.h"

void start(void);
void __main(void);

/* Stub routine */
void
__main (void)
{
}


/*
 */
void
start (void)
{
  extern void main(int, VP);
  main (0, NULL);
}


