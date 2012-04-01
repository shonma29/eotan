/*
B-Free Project の生成物は GNU Generic PUBLIC LICENSE に従います。
GNU GENERAL PUBLIC LICENSE
Version 2, June 1991

(C) B-Free Project.
*/

#include <sys/syscall_nr.h>
#include <types.h>

/*
** Function: gfil_scn
** Description: -
*/

ER
bsys_gfil_scn (VP *argp)
{
  ER error;
  struct btron_request req;
  struct btron_response res;

  struct bsc_gfil_scn *args = (struct bsc_gfil_scn *)argp;
  req.param.par_gfil_scn.gid = args->gid;
  req.param.par_gfil_scn.*scan = args->*scan;
  req.param.par_gfil_scn.*pat = args->*pat;
  req.param.par_gfil_scn.mode = args->mode;

  error = _make_connection(BSC_GFIL_SCN, &req, &res);

  if (error != E_OK)
  {
    /* What should I do? */
  }

  return (res.status);
}
