/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gunc_bmp
** Description: -
*/

ER
bsys_gunc_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gunc_bmp *args = (struct bsc_gunc_bmp *)argp;
  req.param.par_gunc_bmp.gid = args->gid;
  req.param.par_gunc_bmp.dr = args->dr;
  req.param.par_gunc_bmp.cbmp = args->cbmp;
  req.param.par_gunc_bmp.sr = args->sr;
  req.param.par_gunc_bmp.mode = args->mode;

  error = _make_connection(BSC_GUNC_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
