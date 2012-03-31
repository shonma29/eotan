/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gset_lmk
** Description: -
*/

ER
bsys_gset_lmk (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gset_lmk *args = (struct bsc_gset_lmk *)argp;
  req.param.par_gset_lmk.gid = args->gid;
  req.param.par_gset_lmk.nbytes = args->nbytes;
  req.param.par_gset_lmk.*mask = args->*mask;

  error = _make_connection(BSC_GSET_LMK, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
