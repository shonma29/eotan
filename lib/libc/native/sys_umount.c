/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2002, Tomohide Naniwa

*/

#include "../native.h"

extern int strlen(char *);

/* umount */
int
umount (char *special_file)
{
  return (call_lowlib (PSC_UMOUNT,
		       strlen(special_file), special_file));
}


