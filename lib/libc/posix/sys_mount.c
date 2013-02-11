/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2002, Tomohide Naniwa

*/

#include <string.h>
#include "../native.h"
#include "../errno.h"


/* mount */
int
mount (char *special_file, char *dir, int rwflag, char *fstype)
{
  ER			error;
  struct posix_request	req;
  struct posix_response	*res = (struct posix_response*)&req;

  req.param.par_mount.devnamelen = strlen(special_file);
  req.param.par_mount.devname = special_file;
  req.param.par_mount.dirnamelen = strlen(dir);
  req.param.par_mount.dirname = dir;
  req.param.par_mount.fstypelen = strlen(fstype);
  req.param.par_mount.fstype = fstype;
  req.param.par_mount.option = rwflag;

  error = _make_connection(PSC_MOUNT, &req);

  if (error != E_OK) {
      /* What should I do? */
  }
  else if (res->errno) {
    errno = res->errno;
    return (-1);
  }

  return (res->status);
}
