/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_atr
** Description: -
*/

ER
bsys_mchg_atr (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mchg_atr *args = (struct bsc_mchg_atr *)argp;
  req.param.par_mchg_atr.mid = args->mid;
  req.param.par_mchg_atr.selnum = args->selnum;
  req.param.par_mchg_atr.mode = args->mode;

  error = _make_connection(BSC_MCHG_ATR, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
