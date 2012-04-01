/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfra_pol
** Description: -
*/

ER
bsys_gfra_pol (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfra_pol *args = (struct bsc_gfra_pol *)argp;
  req.param.par_gfra_pol.gid = args->gid;
  req.param.par_gfra_pol.*p = args->*p;
  req.param.par_gfra_pol.attr = args->attr;
  req.param.par_gfra_pol.*pat = args->*pat;
  req.param.par_gfra_pol.mode = args->mode;

  error = _make_connection(BSC_GFRA_POL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
