/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_arc
** Description: -
*/

ER
bsys_gdra_arc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_arc *args = (struct bsc_gdra_arc *)argp;
  req.param.par_gdra_arc.gid = args->gid;
  req.param.par_gdra_arc.r = args->r;
  req.param.par_gdra_arc.sp = args->sp;
  req.param.par_gdra_arc.ep = args->ep;
  req.param.par_gdra_arc.attr = args->attr;
  req.param.par_gdra_arc.*pat = args->*pat;
  req.param.par_gdra_arc.angle = args->angle;
  req.param.par_gdra_arc.mode = args->mode;

  error = _make_connection(BSC_GDRA_ARC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
