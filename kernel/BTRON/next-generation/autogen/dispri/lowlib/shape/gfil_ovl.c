/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_ovl
** Description: -
*/

ER
bsys_gfil_ovl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_ovl *args = (struct bsc_gfil_ovl *)argp;
  req.param.par_gfil_ovl.gid = args->gid;
  req.param.par_gfil_ovl.r = args->r;
  req.param.par_gfil_ovl.*pat = args->*pat;
  req.param.par_gfil_ovl.angle = args->angle;
  req.param.par_gfil_ovl.mode = args->mode;

  error = _make_connection(BSC_GFIL_OVL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
