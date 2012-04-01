/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_rgn
** Description: -
*/

ER
bsys_gfil_rgn (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_rgn *args = (struct bsc_gfil_rgn *)argp;
  req.param.par_gfil_rgn.gid = args->gid;
  req.param.par_gfil_rgn.val = args->val;
  req.param.par_gfil_rgn.p = args->p;
  req.param.par_gfil_rgn.*pat = args->*pat;
  req.param.par_gfil_rgn.mode = args->mode;

  error = _make_connection(BSC_GFIL_RGN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
