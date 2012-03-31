/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wrsz_drg
** Description: -
*/

ER
bsys_wrsz_drg (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wrsz_drg *args = (struct bsc_wrsz_drg *)argp;
  req.param.par_wrsz_drg.*evt = args->*evt;
  req.param.par_wrsz_drg.limit = args->limit;
  req.param.par_wrsz_drg.new = args->new;

  error = _make_connection(BSC_WRSZ_DRG, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
