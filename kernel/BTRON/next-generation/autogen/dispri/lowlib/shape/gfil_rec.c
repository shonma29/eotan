/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_rec
** Description: -
*/

ER
bsys_gfil_rec (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_rec *args = (struct bsc_gfil_rec *)argp;
  req.param.par_gfil_rec.gid = args->gid;
  req.param.par_gfil_rec.r = args->r;
  req.param.par_gfil_rec.*pat = args->*pat;
  req.param.par_gfil_rec.angle = args->angle;
  req.param.par_gfil_rec.mode = args->mode;

  error = _make_connection(BSC_GFIL_REC, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
