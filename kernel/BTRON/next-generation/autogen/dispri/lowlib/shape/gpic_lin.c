/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_lin
** Description: -
*/

ER
bsys_gpic_lin (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_lin *args = (struct bsc_gpic_lin *)argp;
  req.param.par_gpic_lin.pt = args->pt;
  req.param.par_gpic_lin.p0 = args->p0;
  req.param.par_gpic_lin.p1 = args->p1;
  req.param.par_gpic_lin.width = args->width;

  error = _make_connection(BSC_GPIC_LIN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
