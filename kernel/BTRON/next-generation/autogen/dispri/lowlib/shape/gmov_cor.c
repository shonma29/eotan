/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gmov_cor
** Description: -
*/

ER
bsys_gmov_cor (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gmov_cor *args = (struct bsc_gmov_cor *)argp;
  req.param.par_gmov_cor.gid = args->gid;
  req.param.par_gmov_cor.dh = args->dh;
  req.param.par_gmov_cor.dv = args->dv;

  error = _make_connection(BSC_GMOV_COR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
