/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_rrc
** Description: -
*/

ER
bsys_gfra_rrc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfra_rrc *args = (struct bsc_gfra_rrc *)argp;
  req.param.par_gfra_rrc.gid = args->gid;
  req.param.par_gfra_rrc.r = args->r;
  req.param.par_gfra_rrc.rh = args->rh;
  req.param.par_gfra_rrc.rv = args->rv;
  req.param.par_gfra_rrc.attr = args->attr;
  req.param.par_gfra_rrc.*pat = args->*pat;
  req.param.par_gfra_rrc.angle = args->angle;
  req.param.par_gfra_rrc.mode = args->mode;

  error = _make_connection(BSC_GFRA_RRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
