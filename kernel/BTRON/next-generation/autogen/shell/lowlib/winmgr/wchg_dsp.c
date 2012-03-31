/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchg_dsp
** Description: -
*/

ER
bsys_wchg_dsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wchg_dsp *args = (struct bsc_wchg_dsp *)argp;
  req.param.par_wchg_dsp.*atr = args->*atr;
  req.param.par_wchg_dsp.*bgpat = args->*bgpat;

  error = _make_connection(BSC_WCHG_DSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
