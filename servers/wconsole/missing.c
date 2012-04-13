/*

  B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2001-2003, Tomohide Naniwa
*/

#include "wconsole_int.h"

#define INC(p,x)        (((W)p) = (((W)p) + sizeof (x)))
printf(B *fmt,...)
{
#if 0
  VP arg0;

  arg0 = (VP)&fmt;
  INC (arg0, B *);
  return (dbg_printf (fmt, (VP)arg0));
#endif
}

W isspace(W ch)
{
  if ((ch == ' ') || (ch == '\t') || (ch == '\n'))
    return (TRUE);
  return (FALSE);
}

W isnum(W ch)
{
  ch -= '0';
  if (ch < 0 || ch > 9) {
    return FALSE;
  }
  return TRUE;
}

int atoi(char *s)
{
  int   result;
  BOOL  minus = FALSE;

  result = 0;
  if (*s == '-') {
    s++;
    minus = TRUE;
  }
  while (isnum(*s)) {
    result = (result * 10) + (*s - '0');
    s++;
  }
  return (minus ? -result: result);
}
