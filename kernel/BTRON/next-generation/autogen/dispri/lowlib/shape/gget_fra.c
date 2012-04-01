/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_fra
** Description: -
*/

ER
bsys_gget_fra (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_fra *args = (struct bsc_gget_fra *)argp;
  req.param.par_gget_fra.gid = args->gid;
  req.param.par_gget_fra.rp = args->rp;

  error = _make_connection(BSC_GGET_FRA, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
