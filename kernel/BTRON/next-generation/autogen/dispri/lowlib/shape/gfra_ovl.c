/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_ovl
** Description: -
*/

ER
bsys_gfra_ovl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfra_ovl *args = (struct bsc_gfra_ovl *)argp;
  req.param.par_gfra_ovl.gid = args->gid;
  req.param.par_gfra_ovl.r = args->r;
  req.param.par_gfra_ovl.attr = args->attr;
  req.param.par_gfra_ovl.*pat = args->*pat;
  req.param.par_gfra_ovl.angle = args->angle;
  req.param.par_gfra_ovl.mode = args->mode;

  error = _make_connection(BSC_GFRA_OVL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
