/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_stw
** Description: -
*/

ER
bsys_gget_stw (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_stw *args = (struct bsc_gget_stw *)argp;
  req.param.par_gget_stw.gid = args->gid;
  req.param.par_gget_stw.*str = args->*str;
  req.param.par_gget_stw.len = args->len;
  req.param.par_gget_stw.*gap = args->*gap;
  req.param.par_gget_stw.*pos = args->*pos;

  error = _make_connection(BSC_GGET_STW, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
