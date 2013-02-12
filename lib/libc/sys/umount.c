/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2002, Tomohide Naniwa

*/

#include <errno.h>
#include <string.h>
#include "posix.h"


/* umount */
int
umount (char *special_file)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_umount.dirnamelen = strlen(special_file);
  req.param.par_umount.dirname = special_file;

  error = _make_connection(PSC_UMOUNT, &req);

  if (error != E_OK) {
      /* What should I do? */
  }
  else if (res->errno) {
    errno = res->errno;
    return (-1);
  }

  return (res->status);
}

