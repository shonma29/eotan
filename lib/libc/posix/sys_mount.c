/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2002, Tomohide Naniwa

*/

#include "../native.h"

extern int strlen(char *);

/* mount */
int
mount (char *special_file, char *dir, int rwflag, char *fstype)
{
  return (call_lowlib (PSC_MOUNT,
		       strlen(special_file), special_file,
		       strlen(dir), dir,
		       rwflag,
		       strlen(fstype), fstype)); 
}


