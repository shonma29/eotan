/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_sec
** Description: -
*/

ER
bsys_gfra_sec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfra_sec *args = (struct bsc_gfra_sec *)argp;
  req.param.par_gfra_sec.gid = args->gid;
  req.param.par_gfra_sec.r = args->r;
  req.param.par_gfra_sec.sp = args->sp;
  req.param.par_gfra_sec.ep = args->ep;
  req.param.par_gfra_sec.attr = args->attr;
  req.param.par_gfra_sec.*pat = args->*pat;
  req.param.par_gfra_sec.angle = args->angle;
  req.param.par_gfra_sec.mode = args->mode;

  error = _make_connection(BSC_GFRA_SEC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
