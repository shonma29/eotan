/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2002, Tomohide Naniwa

*/

#include "sys.h"


/* umount */
int
umount (char *special_file)
{
  struct posix_request	req;

  req.param.par_umount.dirname = special_file;

  return _call_fs(PSC_UMOUNT, &req);
}

