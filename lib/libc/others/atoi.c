/*

B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.

*/

/* $Id: atoi.c,v 1.1 1998/12/12 05:36:51 monaka Exp $ */

#include <core/types.h>
#include <string.h>
#include <ctype.h>

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

  while (isdigit(*s))
    {
      result = (result * 10) + (*s - '0');
      s++;
    }
  return (minus ? -result: result);
}




