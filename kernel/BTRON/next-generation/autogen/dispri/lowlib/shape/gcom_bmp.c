/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gcom_bmp
** Description: -
*/

ER
bsys_gcom_bmp (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gcom_bmp *args = (struct bsc_gcom_bmp *)argp;
  req.param.par_gcom_bmp.gid = args->gid;
  req.param.par_gcom_bmp.dr = args->dr;
  req.param.par_gcom_bmp.cbmp = args->cbmp;
  req.param.par_gcom_bmp.stat = args->stat;

  error = _make_connection(BSC_GCOM_BMP, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
