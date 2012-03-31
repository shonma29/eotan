/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_msk
** Description: -
*/

ER
bsys_gget_msk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_msk *args = (struct bsc_gget_msk *)argp;
  req.param.par_gget_msk.gid = args->gid;
  req.param.par_gget_msk.pixmask = args->pixmask;

  error = _make_connection(BSC_GGET_MSK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
