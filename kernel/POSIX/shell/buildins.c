/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/
/* $Header: /usr/local/src/master/B-Free/Program/btron-pc/kernel/POSIX/shell/buildins.c,v 1.1 1996/11/25 14:03:56 night Exp $ */


static char rcsid[] = "@(#)$Id: buildins.c,v 1.1 1996/11/25 14:03:56 night Exp $";


/*
 * $Log: buildins.c,v $
 * Revision 1.1  1996/11/25 14:03:56  night
 * �ǽ����Ͽ
 *
 *
 */

#include "shell.h"
#include "shell_funcs.h"


struct shell_command buildins[] =
{
  { "pwd",	f_pwd },
  { "cd",	f_cd },
  { "exit",	f_exit },
  { "version",	f_version },
  { "if",	f_if },
  { "while",	f_while },
};




/* f_pwd
 *
 */
int
f_pwd (int ac, char **av)
{
}


/* f_cd
 *
 */
int
f_cd (int ac, char **av)
{
}

/* f_if
 *
 */
int
f_if (int ac, char **av)
{
}

/* f_while
 *
 */
int
f_while (int ac, char **av)
{
}

  

