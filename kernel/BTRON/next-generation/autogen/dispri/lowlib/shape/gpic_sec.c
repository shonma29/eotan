/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_sec
** Description: -
*/

ER
bsys_gpic_sec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_sec *args = (struct bsc_gpic_sec *)argp;
  req.param.par_gpic_sec.pt = args->pt;
  req.param.par_gpic_sec.r = args->r;
  req.param.par_gpic_sec.sp = args->sp;
  req.param.par_gpic_sec.ep = args->ep;
  req.param.par_gpic_sec.angle = args->angle;
  req.param.par_gpic_sec.width = args->width;

  error = _make_connection(BSC_GPIC_SEC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
