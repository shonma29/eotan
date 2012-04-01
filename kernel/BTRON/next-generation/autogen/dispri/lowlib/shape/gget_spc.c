/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_spc
** Description: -
*/

ER
bsys_gget_spc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_spc *args = (struct bsc_gget_spc *)argp;
  req.param.par_gget_spc.*dev = args->*dev;
  req.param.par_gget_spc.*buf = args->*buf;

  error = _make_connection(BSC_GGET_SPC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
