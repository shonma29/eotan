/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_stp
** Description: -
*/

ER
bsys_gdra_stp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_stp *args = (struct bsc_gdra_stp *)argp;
  req.param.par_gdra_stp.gid = args->gid;
  req.param.par_gdra_stp.h = args->h;
  req.param.par_gdra_stp.v = args->v;
  req.param.par_gdra_stp.*str = args->*str;
  req.param.par_gdra_stp.len = args->len;
  req.param.par_gdra_stp.mode = args->mode;

  error = _make_connection(BSC_GDRA_STP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
