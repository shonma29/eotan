/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wend_dsp
** Description: -
*/

ER
bsys_wend_dsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wend_dsp *args = (struct bsc_wend_dsp *)argp;
  req.param.par_wend_dsp.wid = args->wid;

  error = _make_connection(BSC_WEND_DSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
