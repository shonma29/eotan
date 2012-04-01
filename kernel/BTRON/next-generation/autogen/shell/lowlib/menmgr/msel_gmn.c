/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: msel_gmn
** Description: -
*/

ER
bsys_msel_gmn (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_msel_gmn *args = (struct bsc_msel_gmn *)argp;
  req.param.par_msel_gmn.mid = args->mid;
  req.param.par_msel_gmn.pos = args->pos;

  error = _make_connection(BSC_MSEL_GMN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
