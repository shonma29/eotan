/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_sec
** Description: -
*/

ER
bsys_gfil_sec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_sec *args = (struct bsc_gfil_sec *)argp;
  req.param.par_gfil_sec.gid = args->gid;
  req.param.par_gfil_sec.r = args->r;
  req.param.par_gfil_sec.sp = args->sp;
  req.param.par_gfil_sec.ep = args->ep;
  req.param.par_gfil_sec.*pat = args->*pat;
  req.param.par_gfil_sec.angle = args->angle;
  req.param.par_gfil_sec.mode = args->mode;

  error = _make_connection(BSC_GFIL_SEC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
