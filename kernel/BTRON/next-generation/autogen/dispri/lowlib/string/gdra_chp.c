/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_chp
** Description: -
*/

ER
bsys_gdra_chp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_chp *args = (struct bsc_gdra_chp *)argp;
  req.param.par_gdra_chp.gid = args->gid;
  req.param.par_gdra_chp.h = args->h;
  req.param.par_gdra_chp.v = args->v;
  req.param.par_gdra_chp.ch = args->ch;
  req.param.par_gdra_chp.mode = args->mode;

  error = _make_connection(BSC_GDRA_CHP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
