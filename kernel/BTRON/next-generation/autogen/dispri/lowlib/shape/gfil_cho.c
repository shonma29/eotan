/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_cho
** Description: -
*/

ER
bsys_gfil_cho (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_cho *args = (struct bsc_gfil_cho *)argp;
  req.param.par_gfil_cho.gid = args->gid;
  req.param.par_gfil_cho.r = args->r;
  req.param.par_gfil_cho.sp = args->sp;
  req.param.par_gfil_cho.ep = args->ep;
  req.param.par_gfil_cho.*pat = args->*pat;
  req.param.par_gfil_cho.angle = args->angle;
  req.param.par_gfil_cho.mode = args->mode;

  error = _make_connection(BSC_GFIL_CHO, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
