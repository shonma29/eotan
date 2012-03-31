/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mopn_gmn
** Description: -
*/

ER
bsys_mopn_gmn (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mopn_gmn *args = (struct bsc_mopn_gmn *)argp;
  req.param.par_mopn_gmn.dnum = args->dnum;

  error = _make_connection(BSC_MOPN_GMN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
