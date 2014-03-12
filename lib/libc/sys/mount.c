/*

  GNU GENERAL PUBLIC LICENSE
  Version 2, June 1991

  (C) 2002, Tomohide Naniwa

*/

#include <string.h>
#include "sys.h"


/* mount */
int
mount (char *special_file, char *dir, int rwflag, char *fstype)
{
  struct posix_request	req;

  req.param.par_mount.devnamelen = strlen(special_file);
  req.param.par_mount.devname = special_file;
  req.param.par_mount.dirnamelen = strlen(dir);
  req.param.par_mount.dirname = dir;
  req.param.par_mount.fstypelen = strlen(fstype);
  req.param.par_mount.fstype = fstype;
  req.param.par_mount.option = rwflag;

  return _call_fs(PSC_MOUNT, &req);
}
