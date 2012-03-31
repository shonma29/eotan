/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wmov_drg
** Description: -
*/

ER
bsys_wmov_drg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wmov_drg *args = (struct bsc_wmov_drg *)argp;
  req.param.par_wmov_drg.*evt = args->*evt;
  req.param.par_wmov_drg.new = args->new;

  error = _make_connection(BSC_WMOV_DRG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
