/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gsav_bmp
** Description: -
*/

ER
bsys_gsav_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gsav_bmp *args = (struct bsc_gsav_bmp *)argp;
  req.param.par_gsav_bmp.gid = args->gid;
  req.param.par_gsav_bmp.gr = args->gr;
  req.param.par_gsav_bmp.bmap = args->bmap;
  req.param.par_gsav_bmp.br = args->br;
  req.param.par_gsav_bmp.*mask = args->*mask;
  req.param.par_gsav_bmp.mode = args->mode;

  error = _make_connection(BSC_GSAV_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
