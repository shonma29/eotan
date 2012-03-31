/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gget_bmp
** Description: -
*/

ER
bsys_gget_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gget_bmp *args = (struct bsc_gget_bmp *)argp;
  req.param.par_gget_bmp.gid = args->gid;
  req.param.par_gget_bmp.bmap = args->bmap;

  error = _make_connection(BSC_GGET_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
