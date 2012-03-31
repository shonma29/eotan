/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_bnd
** Description: -
*/

ER
bsys_gget_bnd (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_bnd *args = (struct bsc_gget_bnd *)argp;
  req.param.par_gget_bnd.gid = args->gid;
  req.param.par_gget_bnd.rp = args->rp;

  error = _make_connection(BSC_GGET_BND, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
