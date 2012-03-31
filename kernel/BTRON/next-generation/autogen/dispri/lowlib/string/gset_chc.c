/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_chc
** Description: -
*/

ER
bsys_gset_chc (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_chc *args = (struct bsc_gset_chc *)argp;
  req.param.par_gset_chc.gid = args->gid;
  req.param.par_gset_chc.chfgc = args->chfgc;
  req.param.par_gset_chc.chbgc = args->chbgc;

  error = _make_connection(BSC_GSET_CHC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
