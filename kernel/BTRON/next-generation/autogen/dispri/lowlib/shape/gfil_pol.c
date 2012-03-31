/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_pol
** Description: -
*/

ER
bsys_gfil_pol (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_pol *args = (struct bsc_gfil_pol *)argp;
  req.param.par_gfil_pol.gid = args->gid;
  req.param.par_gfil_pol.*p = args->*p;
  req.param.par_gfil_pol.*pat = args->*pat;
  req.param.par_gfil_pol.mode = args->mode;

  error = _make_connection(BSC_GFIL_POL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
