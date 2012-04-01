/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_chc
** Description: -
*/

ER
bsys_gget_chc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_chc *args = (struct bsc_gget_chc *)argp;
  req.param.par_gget_chc.gid = args->gid;
  req.param.par_gget_chc.*chfgc = args->*chfgc;
  req.param.par_gget_chc.*chbgc = args->*chbgc;

  error = _make_connection(BSC_GGET_CHC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
