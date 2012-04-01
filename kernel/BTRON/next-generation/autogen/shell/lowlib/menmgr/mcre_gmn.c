/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mcre_gmn
** Description: -
*/

ER
bsys_mcre_gmn (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mcre_gmn *args = (struct bsc_mcre_gmn *)argp;
  req.param.par_mcre_gmn.*gm = args->*gm;

  error = _make_connection(BSC_MCRE_GMN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
