/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_rrc
** Description: -
*/

ER
bsys_gfil_rrc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_rrc *args = (struct bsc_gfil_rrc *)argp;
  req.param.par_gfil_rrc.gid = args->gid;
  req.param.par_gfil_rrc.r = args->r;
  req.param.par_gfil_rrc.rh = args->rh;
  req.param.par_gfil_rrc.rv = args->rv;
  req.param.par_gfil_rrc.*pat = args->*pat;
  req.param.par_gfil_rrc.angle = args->angle;
  req.param.par_gfil_rrc.mode = args->mode;

  error = _make_connection(BSC_GFIL_RRC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
