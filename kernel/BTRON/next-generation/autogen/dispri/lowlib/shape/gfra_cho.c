/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_cho
** Description: -
*/

ER
bsys_gfra_cho (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfra_cho *args = (struct bsc_gfra_cho *)argp;
  req.param.par_gfra_cho.gid = args->gid;
  req.param.par_gfra_cho.r = args->r;
  req.param.par_gfra_cho.sp = args->sp;
  req.param.par_gfra_cho.ep = args->ep;
  req.param.par_gfra_cho.attr = args->attr;
  req.param.par_gfra_cho.*pat = args->*pat;
  req.param.par_gfra_cho.angle = args->angle;
  req.param.par_gfra_cho.mode = args->mode;

  error = _make_connection(BSC_GFRA_CHO, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
