/*

B-Free Project ������ʪ�� GNU Generic PUBLIC LICENSE �˽����ޤ���

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Id: atoi.c,v 1.1 1998/12/12 05:36:51 monaka Exp $ */
static char rcsid[] = "$Id: atoi.c,v 1.1 1998/12/12 05:36:51 monaka Exp $";

#include <types.h>
#include <string.h>

int
atoi (char *s)
{
  int	result;
  BOOL	minus = FALSE;

  result = 0;
  if (*s == '-')
    {
      s++;
      minus = TRUE;
    }

  while (isnum(*s))
    {
      result = (result * 10) + (*s - '0');
      s++;
    }
  return (minus ? -result: result);
}




