/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_sth
** Description: -
*/

ER
bsys_gget_sth (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_sth *args = (struct bsc_gget_sth *)argp;
  req.param.par_gget_sth.gid = args->gid;
  req.param.par_gget_sth.*str = args->*str;
  req.param.par_gget_sth.len = args->len;
  req.param.par_gget_sth.*cap = args->*cap;
  req.param.par_gget_sth.*pos = args->*pos;

  error = _make_connection(BSC_GGET_STH, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
