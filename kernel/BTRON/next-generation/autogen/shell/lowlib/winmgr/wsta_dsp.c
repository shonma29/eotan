/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wsta_dsp
** Description: -
*/

ER
bsys_wsta_dsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wsta_dsp *args = (struct bsc_wsta_dsp *)argp;
  req.param.par_wsta_dsp.wid = args->wid;
  req.param.par_wsta_dsp.r = args->r;
  req.param.par_wsta_dsp.rlst = args->rlst;

  error = _make_connection(BSC_WSTA_DSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
