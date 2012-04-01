/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_pln
** Description: -
*/

ER
bsys_gdra_pln (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_pln *args = (struct bsc_gdra_pln *)argp;
  req.param.par_gdra_pln.gid = args->gid;
  req.param.par_gdra_pln.*p = args->*p;
  req.param.par_gdra_pln.attr = args->attr;
  req.param.par_gdra_pln.*pat = args->*pat;
  req.param.par_gdra_pln.mode = args->mode;

  error = _make_connection(BSC_GDRA_PLN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
