/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gdra_spl
** Description: -
*/

ER
bsys_gdra_spl (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gdra_spl *args = (struct bsc_gdra_spl *)argp;
  req.param.par_gdra_spl.gid = args->gid;
  req.param.par_gdra_spl.np = args->np;
  req.param.par_gdra_spl.pt = args->pt;
  req.param.par_gdra_spl.attr = args->attr;
  req.param.par_gdra_spl.*pat = args->*pat;
  req.param.par_gdra_spl.mode = args->mode;

  error = _make_connection(BSC_GDRA_SPL, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
