/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: grot_bmp
** Description: -
*/

ER
bsys_grot_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_grot_bmp *args = (struct bsc_grot_bmp *)argp;
  req.param.par_grot_bmp.srcid = args->srcid;
  req.param.par_grot_bmp.dstid = args->dstid;
  req.param.par_grot_bmp.rp = args->rp;
  req.param.par_grot_bmp.dp = args->dp;
  req.param.par_grot_bmp.angle = args->angle;
  req.param.par_grot_bmp.*mask = args->*mask;
  req.param.par_grot_bmp.mode = args->mode;

  error = _make_connection(BSC_GROT_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
