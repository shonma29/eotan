/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wreq_dsp
** Description: -
*/

ER
bsys_wreq_dsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wreq_dsp *args = (struct bsc_wreq_dsp *)argp;
  req.param.par_wreq_dsp.wid = args->wid;

  error = _make_connection(BSC_WREQ_DSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
