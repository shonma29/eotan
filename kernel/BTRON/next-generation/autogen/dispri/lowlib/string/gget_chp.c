/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_chp
** Description: -
*/

ER
bsys_gget_chp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_chp *args = (struct bsc_gget_chp *)argp;
  req.param.par_gget_chp.gid = args->gid;
  req.param.par_gget_chp.*h = args->*h;
  req.param.par_gget_chp.*v = args->*v;

  error = _make_connection(BSC_GGET_CHP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
