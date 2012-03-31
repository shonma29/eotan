/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_ovl
** Description: -
*/

ER
bsys_gpic_ovl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_ovl *args = (struct bsc_gpic_ovl *)argp;
  req.param.par_gpic_ovl.pt = args->pt;
  req.param.par_gpic_ovl.r = args->r;
  req.param.par_gpic_ovl.angle = args->angle;
  req.param.par_gpic_ovl.width = args->width;

  error = _make_connection(BSC_GPIC_OVL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
