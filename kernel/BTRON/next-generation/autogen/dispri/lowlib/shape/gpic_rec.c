/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gpic_rec
** Description: -
*/

ER
bsys_gpic_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gpic_rec *args = (struct bsc_gpic_rec *)argp;
  req.param.par_gpic_rec.pt = args->pt;
  req.param.par_gpic_rec.r = args->r;
  req.param.par_gpic_rec.angle = args->angle;
  req.param.par_gpic_rec.width = args->width;

  error = _make_connection(BSC_GPIC_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
