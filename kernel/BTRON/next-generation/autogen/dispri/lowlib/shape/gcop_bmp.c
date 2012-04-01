/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcop_bmp
** Description: -
*/

ER
bsys_gcop_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gcop_bmp *args = (struct bsc_gcop_bmp *)argp;
  req.param.par_gcop_bmp.srcid = args->srcid;
  req.param.par_gcop_bmp.sr = args->sr;
  req.param.par_gcop_bmp.dstid = args->dstid;
  req.param.par_gcop_bmp.dr = args->dr;
  req.param.par_gcop_bmp.*mask = args->*mask;
  req.param.par_gcop_bmp.mode = args->mode;

  error = _make_connection(BSC_GCOP_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
