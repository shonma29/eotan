/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_chp
** Description: -
*/

ER
bsys_gset_chp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_chp *args = (struct bsc_gset_chp *)argp;
  req.param.par_gset_chp.gid = args->gid;
  req.param.par_gset_chp.h = args->h;
  req.param.par_gset_chp.v = args->v;
  req.param.par_gset_chp.abs = args->abs;

  error = _make_connection(BSC_GSET_CHP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
