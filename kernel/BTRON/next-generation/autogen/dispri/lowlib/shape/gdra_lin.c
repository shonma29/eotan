/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_lin
** Description: -
*/

ER
bsys_gdra_lin (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_lin *args = (struct bsc_gdra_lin *)argp;
  req.param.par_gdra_lin.gid = args->gid;
  req.param.par_gdra_lin.p1 = args->p1;
  req.param.par_gdra_lin.p2 = args->p2;
  req.param.par_gdra_lin.attr = args->attr;
  req.param.par_gdra_lin.*pat = args->*pat;
  req.param.par_gdra_lin.mode = args->mode;

  error = _make_connection(BSC_GDRA_LIN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
