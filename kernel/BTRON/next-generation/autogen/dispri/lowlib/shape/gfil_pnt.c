/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_pnt
** Description: -
*/

ER
bsys_gfil_pnt (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_pnt *args = (struct bsc_gfil_pnt *)argp;
  req.param.par_gfil_pnt.gid = args->gid;
  req.param.par_gfil_pnt.p = args->p;
  req.param.par_gfil_pnt.*pat = args->*pat;
  req.param.par_gfil_pnt.mode = args->mode;

  error = _make_connection(BSC_GFIL_PNT, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
