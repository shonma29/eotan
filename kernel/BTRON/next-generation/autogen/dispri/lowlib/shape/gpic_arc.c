/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_arc
** Description: -
*/

ER
bsys_gpic_arc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_arc *args = (struct bsc_gpic_arc *)argp;
  req.param.par_gpic_arc.pt = args->pt;
  req.param.par_gpic_arc.r = args->r;
  req.param.par_gpic_arc.sp = args->sp;
  req.param.par_gpic_arc.ep = args->ep;
  req.param.par_gpic_arc.angle = args->angle;
  req.param.par_gpic_arc.width = args->width;

  error = _make_connection(BSC_GPIC_ARC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
