#include "stdarg.h"
#include "string.h"
#include "console.h"


int
boot_printf (char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    return vnprintf((void*)(putchar), (char*)format, ap);
}

void
busywait(int x)
{
  int i,j;
  int	tmp;

  for (i = 0; i < x; i++)
     for (j = 0; j < 100; j++)
       tmp = j;
}
