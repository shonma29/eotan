/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_cho
** Description: -
*/

ER
bsys_gpic_cho (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_cho *args = (struct bsc_gpic_cho *)argp;
  req.param.par_gpic_cho.pt = args->pt;
  req.param.par_gpic_cho.r = args->r;
  req.param.par_gpic_cho.sp = args->sp;
  req.param.par_gpic_cho.ep = args->ep;
  req.param.par_gpic_cho.angle = args->angle;
  req.param.par_gpic_cho.width = args->width;

  error = _make_connection(BSC_GPIC_CHO, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
