/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_msk
** Description: -
*/

ER
bsys_gset_msk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_msk *args = (struct bsc_gset_msk *)argp;
  req.param.par_gset_msk.gid = args->gid;
  req.param.par_gset_msk.pixmask = args->pixmask;

  error = _make_connection(BSC_GSET_MSK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
