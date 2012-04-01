/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmov_bnd
** Description: -
*/

ER
bsys_gmov_bnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gmov_bnd *args = (struct bsc_gmov_bnd *)argp;
  req.param.par_gmov_bnd.gid = args->gid;
  req.param.par_gmov_bnd.dh = args->dh;
  req.param.par_gmov_bnd.dv = args->dv;

  error = _make_connection(BSC_GMOV_BND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
