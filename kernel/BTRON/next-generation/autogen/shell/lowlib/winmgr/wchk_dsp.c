/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: wchk_dsp
** Description: -
*/

ER
bsys_wchk_dsp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_wchk_dsp *args = (struct bsc_wchk_dsp *)argp;
  req.param.par_wchk_dsp.wid = args->wid;

  error = _make_connection(BSC_WCHK_DSP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
