/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: grst_bmp
** Description: -
*/

ER
bsys_grst_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_grst_bmp *args = (struct bsc_grst_bmp *)argp;
  req.param.par_grst_bmp.gid = args->gid;
  req.param.par_grst_bmp.gr = args->gr;
  req.param.par_grst_bmp.bmap = args->bmap;
  req.param.par_grst_bmp.br = args->br;
  req.param.par_grst_bmp.*mask = args->*mask;
  req.param.par_grst_bmp.mode = args->mode;

  error = _make_connection(BSC_GRST_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
