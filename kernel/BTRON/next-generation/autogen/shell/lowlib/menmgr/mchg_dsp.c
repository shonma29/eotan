/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: mchg_dsp
** Description: -
*/

ER
bsys_mchg_dsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_mchg_dsp *args = (struct bsc_mchg_dsp *)argp;
  req.param.par_mchg_dsp.*attr = args->*attr;
  req.param.par_mchg_dsp.posattr = args->posattr;

  error = _make_connection(BSC_MCHG_DSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
